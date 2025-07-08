package main

import (
	"bufio"
	"fmt"
	"io"
	"log"
	"os/exec"
	"strings"
	"sync"
	"time"
)

// CppEngineManager handles the communication with the C++ chess engine subprocess.
type CppEngineManager struct {
	cmd    *exec.Cmd
	stdin  io.WriteCloser
	stdout *bufio.Scanner // To read line by line
	stderr *bufio.Scanner // To read error output line by line

	mu sync.Mutex // To serialize access to the C++ process
}

// Global instance of the engine manager
var globalEngineManager *CppEngineManager

// Initializes and starts the C++ engine subprocess.
// This should be called once at application startup.
func InitGlobalEngineManager(executablePath string) error {
	manager, err := NewCppEngineManager(executablePath)
	if err != nil {
		return fmt.Errorf("failed to initialize C++ engine manager: %w", err)
	}
	globalEngineManager = manager
	log.Println("C++ chess engine manager initialized and process started.")
	return nil
}

// NewCppEngineManager starts the C++ engine executable and sets up pipes.
func NewCppEngineManager(executablePath string) (*CppEngineManager, error) {
	cmd := exec.Command(executablePath)

	stdinPipe, err := cmd.StdinPipe()
	if err != nil {
		return nil, fmt.Errorf("failed to get stdin pipe: %w", err)
	}

	stdoutPipe, err := cmd.StdoutPipe()
	if err != nil {
		return nil, fmt.Errorf("failed to get stdout pipe: %w", err)
	}

	stderrPipe, err := cmd.StderrPipe()
	if err != nil {
		return nil, fmt.Errorf("failed to get stderr pipe: %w", err)
	}

	if err := cmd.Start(); err != nil {
		return nil, fmt.Errorf("failed to start C++ engine process '%s': %w", executablePath, err)
	}

	log.Printf("C++ engine process started (PID: %d)\n", cmd.Process.Pid)

	manager := &CppEngineManager{
		cmd:    cmd,
		stdin:  stdinPipe,
		stdout: bufio.NewScanner(stdoutPipe),
        stderr: bufio.NewScanner(stderrPipe),
	}

	// Goroutine to monitor stderr from the C++ process
	go func() {
		for manager.stderr.Scan() {
			log.Printf("C++ Engine STDERR: %s\n", manager.stderr.Text())
		}
		if err := manager.stderr.Err(); err != nil {
            if err != io.EOF { // EOF is expected when process closes pipe
			    log.Printf("Error reading C++ engine stderr: %v\n", err)
            }
		}
        log.Println("C++ Engine STDERR monitoring stopped.")
	}()

    // Goroutine to ensure the process is eventually cleaned up if it exits unexpectedly
    go func() {
        err := cmd.Wait()
        log.Printf("C++ engine process exited with error: %v (if nil, means normal exit or killed by Close)\n", err)
        // Potentially add restart logic here if desired
    }()

	return manager, nil
}

// SendCommand sends a command to the C++ engine and waits for a single line response.
// It ensures thread-safe access to the C++ process.
func (m *CppEngineManager) SendCommand(command string) (string, error) {
	m.mu.Lock()
	defer m.mu.Unlock()

	if m.cmd == nil || m.cmd.ProcessState != nil && m.cmd.ProcessState.Exited() {
		return "", fmt.Errorf("C++ engine process is not running")
	}

	log.Printf("Go -> C++: %s\n", command)
	if _, err := fmt.Fprintln(m.stdin, command); err != nil {
		return "", fmt.Errorf("failed to send command to C++ engine: %w", err)
	}

	// Basic timeout for reading response
	// TODO: Make timeout configurable
	responseChannel := make(chan string)
	errorChannel := make(chan error)

	go func() {
		if m.stdout.Scan() {
			response := m.stdout.Text()
			log.Printf("C++ -> Go: %s\n", response)
			if strings.HasPrefix(response, "ERROR:") {
				errorChannel <- fmt.Errorf("C++ engine error: %s", strings.TrimPrefix(response, "ERROR: "))
				return
			}
			responseChannel <- response
		} else {
			err := m.stdout.Err()
			if err == nil { // Scan returned false but no error, could be EOF
				err = fmt.Errorf("C++ engine closed stdout unexpectedly or no response")
			}
			errorChannel <- fmt.Errorf("failed to read response from C++ engine: %w", err)
		}
	}()

	select {
	case res := <-responseChannel:
		return res, nil
	case err := <-errorChannel:
		return "", err
	case <-time.After(5 * time.Second): // TODO: Adjust timeout as needed, especially for engine thinking time.
                                        // GET_ENGINE_MOVE might need a much longer timeout.
		return "", fmt.Errorf("timeout waiting for response from C++ engine for command: %s", command)
	}
}


// SendCommandWithCustomTimeout sends a command with a specific timeout.
// Useful for commands like GET_ENGINE_MOVE that might take longer.
func (m *CppEngineManager) SendCommandWithCustomTimeout(command string, timeout time.Duration) (string, error) {
	m.mu.Lock()
	defer m.mu.Unlock()

	if m.cmd == nil || m.cmd.ProcessState != nil && m.cmd.ProcessState.Exited() {
		return "", fmt.Errorf("C++ engine process is not running")
	}

	log.Printf("Go -> C++ (timeout %s): %s\n", timeout.String(), command)
	if _, err := fmt.Fprintln(m.stdin, command); err != nil {
		return "", fmt.Errorf("failed to send command to C++ engine: %w", err)
	}

	responseChannel := make(chan string)
	errorChannel := make(chan error)

	go func() {
		if m.stdout.Scan() {
			response := m.stdout.Text()
			log.Printf("C++ -> Go: %s\n", response)
			if strings.HasPrefix(response, "ERROR:") {
				errorChannel <- fmt.Errorf("C++ engine error: %s", strings.TrimPrefix(response, "ERROR: "))
				return
			}
			responseChannel <- response
		} else {
			err := m.stdout.Err()
			if err == nil {
				err = fmt.Errorf("C++ engine closed stdout unexpectedly or no response")
			}
			errorChannel <- fmt.Errorf("failed to read response from C++ engine: %w", err)
		}
	}()

	select {
	case res := <-responseChannel:
		return res, nil
	case err := <-errorChannel:
		return "", err
	case <-time.After(timeout):
		return "", fmt.Errorf("timeout waiting for response from C++ engine for command: %s", command)
	}
}


// Close terminates the C++ engine process.
func (m *CppEngineManager) Close() error {
	m.mu.Lock() // Ensure no commands are being sent during close
	defer m.mu.Unlock()

	log.Println("Attempting to close C++ engine manager...")
	if m.cmd == nil || m.cmd.ProcessState != nil && m.cmd.ProcessState.Exited() {
		log.Println("C++ engine process already exited or not started.")
		return nil
	}

	log.Println("Sending QUIT command to C++ engine...")
	if _, err := fmt.Fprintln(m.stdin, "QUIT"); err != nil {
		log.Printf("Failed to send QUIT command to C++ engine: %v. Attempting to kill process.", err)
		// Proceed to kill if QUIT fails
	} else {
        // Give it a moment to shut down gracefully
        // Note: This is a simple wait; a more robust solution might check if the process actually exited.
        time.Sleep(100 * time.Millisecond)
    }


	if m.cmd.ProcessState == nil || !m.cmd.ProcessState.Exited() {
		log.Println("C++ engine process did not exit after QUIT, attempting to kill...")
		if err := m.cmd.Process.Kill(); err != nil {
			log.Printf("Failed to kill C++ engine process: %v", err)
			return fmt.Errorf("failed to kill C++ engine process: %w", err)
		}
		log.Println("C++ engine process killed.")
	} else {
		log.Println("C++ engine process exited gracefully after QUIT.")
	}

    // Close stdin pipe explicitly, though killing process usually handles this.
    if m.stdin != nil {
        m.stdin.Close()
    }

	// Wait for the process to fully release resources. cmd.Wait() is called in the goroutine.
	log.Println("C++ engine manager closed.")
	return nil
}
