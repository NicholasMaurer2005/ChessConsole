package main

import (
	"crypto/rand"
	"encoding/hex"
	"log"
	"net/http"
	"os"
	"strings"
	"sync"
	"time"

	"github.com/gin-gonic/gin"
)

const (
	DefaultCppEnginePath = "./ChessConsole.exe" // Adjust as needed, or make configurable via env var
	EngineThinkTime      = 30 * time.Second     // Max time for GET_ENGINE_MOVE
	DefaultCommandTimeout = 5 * time.Second     // Default timeout for quick commands
)
// --- Game State Management ---
// For simplicity, using an in-memory map.
// gameID -> currentFEN string
var gameStates = make(map[string]string)
var gameStatesMutex = &sync.RWMutex{}

// Helper to generate unique game IDs
func generateGameID() (string, error) {
	bytes := make([]byte, 16) // 128-bit random ID
	if _, err := rand.Read(bytes); err != nil {
		return "", err
	}
	return hex.EncodeToString(bytes), nil
}

// --- Request/Response Structs ---
type NewGameRequest struct {
	FEN string `json:"fen"` // Optional, defaults to starting position
}

type NewGameResponse struct {
	GameID      string `json:"game_id"`
	Board       string `json:"board"` // FEN string
	Message     string `json:"message,omitempty"`
}

type MakeMoveRequest struct {
	Move string `json:"move"` // e.g., "e2e4"
}

type MakeMoveResponse struct {
	GameID  string `json:"game_id"`
	Board   string `json:"board"` // New FEN string
	Message string `json:"message,omitempty"`
}

type GetEngineMoveResponse struct {
	GameID    string `json:"game_id"`
	EngineMove string `json:"engine_move"` // e.g., "e7e5"
	Board     string `json:"board"`       // New FEN string
	Message   string `json:"message,omitempty"`
}

// --- API Handlers (Placeholders for now) ---

func handleNewGame(c *gin.Context) {
	var req NewGameRequest
	if err := c.ShouldBindJSON(&req); err != nil {
		// If FEN is not provided or JSON is malformed, req.FEN will be empty.
		// We can allow this and use a default FEN.
		if req.FEN == "" {
			// TODO: Get default FEN from C++ engine or use a constant
			// For now, using a well-known default. The C++ engine will also use its default if "DEFAULT" is sent.
			req.FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
		}
	} else if req.FEN == "" {
		// Handle case where JSON binding is fine, but FEN field is explicitly empty
		req.FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
	}

	fenToSend := req.FEN
	if strings.TrimSpace(fenToSend) == "" || strings.ToUpper(strings.TrimSpace(fenToSend)) == "DEFAULT" {
		fenToSend = "DEFAULT" // Let C++ engine use its compiled-in default
	}

	cppCommand := fmt.Sprintf("NEW_GAME %s", fenToSend)
	response, err := globalEngineManager.SendCommandWithCustomTimeout(cppCommand, DefaultCommandTimeout)
	if err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": "Failed to communicate with chess engine: " + err.Error()})
		return
	}

	// Expected response: "FEN: <fen_string>"
	if !strings.HasPrefix(response, "FEN: ") {
		c.JSON(http.StatusInternalServerError, gin.H{"error": "Unexpected response from chess engine: " + response})
		return
	}
	boardFEN := strings.TrimPrefix(response, "FEN: ")

	gameID, err := generateGameID()
	if err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": "Failed to generate game ID"})
		return
	}

	gameStatesMutex.Lock()
	gameStates[gameID] = boardFEN
	gameStatesMutex.Unlock()

	c.JSON(http.StatusOK, NewGameResponse{
		GameID: gameID,
		Board:  boardFEN,
		Message: "New game started.",
	})
}

func handleGetBoard(c *gin.Context) {
	gameID := c.Param("gameID")
	if gameID == "" {
		c.JSON(http.StatusBadRequest, gin.H{"error": "Game ID is required"})
		return
	}

	gameStatesMutex.RLock()
	currentFEN, ok := gameStates[gameID]
	gameStatesMutex.RUnlock()

	if !ok {
		c.JSON(http.StatusNotFound, gin.H{"error": "Game not found"})
		return
	}

	c.JSON(http.StatusOK, gin.H{
		"game_id": gameID,
		"board":   currentFEN,
	})
}

func handleMakeMove(c *gin.Context) {
	gameID := c.Param("gameID")
	if gameID == "" {
		c.JSON(http.StatusBadRequest, gin.H{"error": "Game ID is required"})
		return
	}

	var req MakeMoveRequest
	if err := c.ShouldBindJSON(&req); err != nil || req.Move == "" {
		c.JSON(http.StatusBadRequest, gin.H{"error": "Invalid request: move string is required"})
		return
	}

	gameStatesMutex.RLock()
	currentFEN, ok := gameStates[gameID]
	gameStatesMutex.RUnlock()

	if !ok {
		c.JSON(http.StatusNotFound, gin.H{"error": "Game not found"})
		return
	}

	cppCommand := fmt.Sprintf("MAKE_MOVE %s %s", currentFEN, req.Move)
	response, err := globalEngineManager.SendCommandWithCustomTimeout(cppCommand, DefaultCommandTimeout)
	if err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": "Failed to communicate with chess engine: " + err.Error()})
		return
	}

	// Expected response: "FEN: <new_fen_string>" or "ERROR: <message>"
	if strings.HasPrefix(response, "ERROR:") { // C++ engine itself sent an error (e.g. invalid move)
		c.JSON(http.StatusBadRequest, gin.H{"error": "Chess engine error: " + strings.TrimPrefix(response, "ERROR: ")})
		return
	}
	if !strings.HasPrefix(response, "FEN: ") {
		c.JSON(http.StatusInternalServerError, gin.H{"error": "Unexpected response from chess engine: " + response})
		return
	}
	newBoardFEN := strings.TrimPrefix(response, "FEN: ")

	gameStatesMutex.Lock()
	gameStates[gameID] = newBoardFEN
	gameStatesMutex.Unlock()

	c.JSON(http.StatusOK, MakeMoveResponse{
		GameID: gameID,
		Board:  newBoardFEN,
		Message: "Move processed.",
	})
}

func handleGetEngineMove(c *gin.Context) {
	gameID := c.Param("gameID")
	if gameID == "" {
		c.JSON(http.StatusBadRequest, gin.H{"error": "Game ID is required"})
		return
	}

	gameStatesMutex.RLock()
	currentFEN, ok := gameStates[gameID]
	gameStatesMutex.RUnlock()

	if !ok {
		c.JSON(http.StatusNotFound, gin.H{"error": "Game not found"})
		return
	}

	// Optional: Allow specifying depth in request, otherwise use default from C++
	// For now, C++ engine in Main.cpp uses a default depth for GET_ENGINE_MOVE
	// Or, we could add depth to the command: fmt.Sprintf("GET_ENGINE_MOVE %s %d", currentFEN, depth)
	cppCommand := fmt.Sprintf("GET_ENGINE_MOVE %s", currentFEN)
	response, err := globalEngineManager.SendCommandWithCustomTimeout(cppCommand, EngineThinkTime) // Use longer timeout
	if err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": "Failed to communicate with chess engine for engine move: " + err.Error()})
		return
	}

	// Expected response: "MOVE: <move_str> FEN: <new_fen_string>" or "ERROR: <message>" or "MOVE: NOMOVE FEN: <current_fen>"
	if strings.HasPrefix(response, "ERROR:") {
		c.JSON(http.StatusBadRequest, gin.H{"error": "Chess engine error on get engine move: " + strings.TrimPrefix(response, "ERROR: ")})
		return
	}
	if !strings.HasPrefix(response, "MOVE: ") {
		c.JSON(http.StatusInternalServerError, gin.H{"error": "Unexpected response from chess engine for engine move: " + response})
		return
	}

	parts := strings.Fields(response) // MOVE: e2e4 FEN: rnbqkb...
	if len(parts) < 4 || parts[0] != "MOVE:" || parts[2] != "FEN:" {
		c.JSON(http.StatusInternalServerError, gin.H{"error": "Malformed response from chess engine: " + response})
		return
	}
	engineMove := parts[1]
	newBoardFEN := strings.Join(parts[3:], " ") // FEN can have spaces if not handled by C++ output (should be ok)

    if engineMove == "NOMOVE" {
        // Engine indicated no move was made (e.g., checkmate/stalemate)
        // gameStates FEN is already up-to-date from the last player move.
        // We just return the current state.
        gameStatesMutex.RLock()
        finalFEN := gameStates[gameID] // Should be the same as newBoardFEN if C++ returns current FEN on NOMOVE
        gameStatesMutex.RUnlock()

        c.JSON(http.StatusOK, GetEngineMoveResponse{
            GameID:    gameID,
            EngineMove: engineMove, // "NOMOVE"
            Board:     finalFEN,
            Message:   "Engine indicates no move possible (e.g., game over).",
        })
        return
    }


	gameStatesMutex.Lock()
	gameStates[gameID] = newBoardFEN
	gameStatesMutex.Unlock()

	c.JSON(http.StatusOK, GetEngineMoveResponse{
		GameID:    gameID,
		EngineMove: engineMove,
		Board:     newBoardFEN,
		Message: "Engine move processed.",
	})
}


func main() {
	gin.SetMode(gin.ReleaseMode) // Or gin.DebugMode
	r := gin.Default()

	enginePath := os.Getenv("CHESS_ENGINE_PATH")
	if enginePath == "" {
		enginePath = DefaultCppEnginePath
	}

	if err := InitGlobalEngineManager(enginePath); err != nil {
		log.Fatalf("Failed to initialize C++ engine manager: %v", err)
	}
	defer globalEngineManager.Close()


	// Basic route for testing
	r.GET("/ping", func(c *gin.Context) {
		c.JSON(http.StatusOK, gin.H{
			"message": "pong",
		})
	})

	// API routes
	api := r.Group("/api/v1/chess")
	{
		api.POST("/game", handleNewGame)
		api.GET("/game/:gameID/board", handleGetBoard)
		api.POST("/game/:gameID/move", handleMakeMove)
		api.POST("/game/:gameID/engine_move", handleGetEngineMove)
	}


	// Placeholder for where C++ engine interaction will be managed
	// setupCppEngineManager()

	// Run the server
	// We can make the port configurable later
	if err := r.Run(":8080"); err != nil {
		panic("Failed to run Gin server: " + err.Error())
	}
}
