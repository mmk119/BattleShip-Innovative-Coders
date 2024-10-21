#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define GRID_SIZE 10
#define MAX_SHIPS 5
#define MAX_NAME_LENGTH 50

typedef struct {
    char name[20];
    int size;
    int hits;
    int placed;
} Ship;

typedef struct {
    char grid[GRID_SIZE][GRID_SIZE];
    Ship ships[MAX_SHIPS];
    char name[MAX_NAME_LENGTH];
} Player;

// Function prototypes
void initializeGrid(Player *player);
void displayGrid(Player *player, Player *opponent, int revealShips, int trackMisses);
int placeShip(Player *player, int shipIndex, char *coordinate, char orientation);
int isValidPlacement(Player *player, int shipIndex, int row, int col, char orientation);
void fire(Player *attacker, Player *defender, char *coordinate);
void radarSweep(Player *player, char *coordinate, Player *opponent);
int getRandomPlayer();
void clearScreen();

int main() {
    Player player1, player2;
    int turn = getRandomPlayer();
    char command[50];
    int difficultyLevel;

    // Get difficulty level
    printf("Choose tracking difficulty (1 for easy, 2 for hard): ");
    scanf("%d", &difficultyLevel);
    getchar(); // Consume newline character left by scanf

    // Add player names
    printf("Enter name for Player 1: ");
    fgets(player1.name, MAX_NAME_LENGTH, stdin);
    player1.name[strcspn(player1.name, "\n")] = 0; // Remove newline
    printf("Enter name for Player 2: ");
    fgets(player2.name, MAX_NAME_LENGTH, stdin);
    player2.name[strcspn(player2.name, "\n")] = 0; // Remove newline


    // Initialize players
    //strcpy(player1.name, "Player 1");
    //strcpy(player2.name, "Player 2");

    // Define ships
    Ship ships[MAX_SHIPS] = {
        {"Carrier", 5, 0, 0},
        {"Battleship", 4, 0, 0},
        {"Cruiser", 3, 0, 0},
        {"Submarine", 3, 0, 0},
        {"Destroyer", 2, 0, 0}
    };

    for (int i = 0; i < MAX_SHIPS; i++) {
        player1.ships[i] = ships[i];
        player2.ships[i] = ships[i];
    }

    // Initialize grids
    initializeGrid(&player1);
    initializeGrid(&player2);

    // Ship placement for Player 1
    printf("%s, place your ships:\n", player1.name);
    for (int i = 0; i < MAX_SHIPS; i++) {
        char orientation;
        char coordinate[3];
        displayGrid(&player1, &player2, 1, 1); // Show current grid
        printf("Place %s (size %d): Enter coordinate (e.g., B3) and orientation (h/v): ", player1.ships[i].name, player1.ships[i].size);
        scanf("%s %c", coordinate, &orientation);
        if (placeShip(&player1, i, coordinate, orientation) == 0) {
            i--; // Repeat for invalid placement
        }
        clearScreen();
    }

    // Ship placement for Player 2
    printf("%s, place your ships:\n", player2.name);
    for (int i = 0; i < MAX_SHIPS; i++) {
        char orientation;
        char coordinate[3];
        displayGrid(&player2, &player1, 1, 1); // Show current grid
        printf("Place %s (size %d): Enter coordinate (e.g., B3) and orientation (h/v): ", player2.ships[i].name, player2.ships[i].size);
        scanf("%s %c", coordinate, &orientation);
        if (placeShip(&player2, i, coordinate, orientation) == 0) {
            i--; // Repeat for invalid placement
        }
        clearScreen();
    }

    // Gameplay loop
    while (1) {
        Player *currentPlayer = (turn % 2 == 0) ? &player1 : &player2;
        Player *opponentPlayer = (turn % 2 == 0) ? &player2 : &player1;

        displayGrid(currentPlayer, opponentPlayer, 0, difficultyLevel == 1); // Show opponent's grid, track misses if easy
        printf("%s's turn. Enter command (Fire [coordinate] or Radar [coordinate]): ", currentPlayer->name);
        fgets(command, sizeof(command), stdin);
        command[strcspn(command, "\n")] = 0; // Remove newline

        if (strncmp(command, "Fire", 4) == 0) {
            fire(currentPlayer, opponentPlayer, command + 5);
        } else if (strncmp(command, "Radar", 5) == 0) {
            radarSweep(currentPlayer, command + 6, opponentPlayer);
        } else {
            printf("Invalid command.\n");
            continue;
        }

        // Check for game over
        int allShipsSunk = 1;
        for (int i = 0; i < MAX_SHIPS; i++) {
            if (opponentPlayer->ships[i].hits < opponentPlayer->ships[i].size) {
                allShipsSunk = 0;
                break;
            }
        }

        if (allShipsSunk) {
            printf("%s wins!\n", currentPlayer->name);
            break;
        }

        turn++;
    }

    return 0;
}

void initializeGrid(Player *player) {
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            player->grid[i][j] = '~'; // Initialize grid with water
        }
    }
}

void displayGrid(Player *player, Player *opponent, int revealShips, int trackMisses) {
    printf("  A B C D E F G H I J\n");
    for (int i = 0; i < GRID_SIZE; i++) {
        printf("%d ", i + 1);
        for (int j = 0; j < GRID_SIZE; j++) {
            if (opponent->grid[i][j] == 'H') {
                printf("H "); // Hit
            } else if (opponent->grid[i][j] == 'M' && trackMisses) {
                printf("M "); // Miss
            } else if (revealShips && opponent->grid[i][j] != '~') {
                printf("S "); // Ship revealed
            } else {
                printf("~ "); // Water
            }
        }
        printf(" | ");
        for (int j = 0; j < GRID_SIZE; j++) {
            printf("%c ", player->grid[i][j]); // Display own grid
        }
        printf("\n");
    }
}

int placeShip(Player *player, int shipIndex, char *coordinate, char orientation) {
    int row = coordinate[1] - '1';   // Parse row (1-based to 0-based)
    int col = coordinate[0] - 'A';   // Parse column (A-J to 0-9)

    if (isValidPlacement(player, shipIndex, row, col, orientation)) {
        for (int i = 0; i < player->ships[shipIndex].size; i++) {
            player->grid[row][col] = 'S';  // Use 'S' for all ships
            if (orientation == 'h') {
                col++;
            } else {
                row++;
            }
        }
        player->ships[shipIndex].placed = 1;
        return 1; // Successful placement
    }
    printf("Invalid placement. Try again.\n");
    return 0; // Failed placement
}

int isValidPlacement(Player *player, int shipIndex, int row, int col, char orientation) {
    if (row < 0 || col < 0 || row >= GRID_SIZE || col >= GRID_SIZE) return 0; // Out of bounds
    if (orientation == 'h') {
        if (col + player->ships[shipIndex].size > GRID_SIZE) return 0; // Out of bounds horizontally
        for (int i = 0; i < player->ships[shipIndex].size; i++) {
            if (player->grid[row][col + i] != '~') return 0; // Space occupied
        }
    } else {
        if (row + player->ships[shipIndex].size > GRID_SIZE) return 0; // Out of bounds vertically
        for (int i = 0; i < player->ships[shipIndex].size; i++) {
            if (player->grid[row + i][col] != '~') return 0; // Space occupied
        }
    }
    return 1; // Valid placement
}

void fire(Player *attacker, Player *defender, char *coordinate) {
       int row = coordinate[1] - '1';
       int col = coordinate[0] - 'A';
       if (row < 0 || col < 0 || row >= GRID_SIZE || col >= GRID_SIZE) {
           printf("Invalid coordinates. Missed!\n");
           return;
       }
       if (defender->grid[row][col] != '~' && defender->grid[row][col] != 'H') {
           printf("Hit!\n");
           char shipChar = defender->grid[row][col]; // Save the char of the ship hit before overwriting
           defender->grid[row][col] = 'H';
           for (int i = 0; i < MAX_SHIPS; i++) {
               if (defender->ships[i].name[0] == shipChar) {
                   defender->ships[i].hits++;
                   break;
               }
           }
       } else {
           printf("Miss!\n");
           defender->grid[row][col] = 'M';
       }
   }

void radarSweep(Player *player, char *coordinate, Player *opponent) {
    int row = coordinate[1] - '1';
    int col = coordinate[0] - 'A';
    if (row < 0 || col < 0 || row >= GRID_SIZE || col >= GRID_SIZE) {
        printf("Invalid coordinates for radar sweep.\n");
        return;
    }
    printf("Radar sweep at %s...\n", coordinate);
    // Reveal the grid of the opponent for that area
    for (int i = row - 1; i <= row + 1; i++) {
        for (int j = col - 1; j <= col + 1; j++) {
            if (i >= 0 && i < GRID_SIZE && j >= 0 && j < GRID_SIZE) {
                if (opponent->grid[i][j] == 'S') {
                    printf("Ship detected at %c%d!\n", 'A' + j, i + 1);
                }
            }
        }
    }
}

int getRandomPlayer() {
    srand(time(NULL));
    return rand() % 2; // Randomly return 0 or 1
}

void clearScreen() {
       // System call to clear the console
       #ifdef _WIN32
           system("cls");
       #else
           system("clear");
       #endif
   }
