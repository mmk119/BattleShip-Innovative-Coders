#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define GRID_SIZE 10
#define MAX_SHIPS 5
#define MAX_NAME_LENGTH 50
#define MAX_SMOKES 3

typedef struct {
    char name[20];
    int size;
    int hits;
    int placed;
    int isSunk;
} Ship;

typedef struct {
    char grid[GRID_SIZE][GRID_SIZE];
    Ship ships[MAX_SHIPS];
    char name[MAX_NAME_LENGTH];
    int radarSweeps;
    int SmokeScreen;
    int ShipsSunk;
} Player;

// Function prototypes
void initializeGrid(Player *player);
void displayGrid(Player *player, Player *opponent, int revealShips, int trackMisses);
int placeShip(Player *player, int shipIndex, char *coordinate, char orientation);
int isValidPlacement(Player *player, int shipIndex, int row, int col, char orientation);
void fire(Player *attacker, Player *defender, char *coordinate);
void radarSweep(Player *player, char *coordinate, Player *opponent);
void SmokeScreen(Player *player, char *coordinate, Player *opponent);
void checkSunkShips(Player *attacker, Player *defender);
int getRandomPlayer();
void clearScreen();

int main() {
    Player player1, player2;
    int turn = getRandomPlayer(); 
    char command[50];
    int difficultyLevel;

    player1.radarSweeps = 0;
    player2.radarSweeps = 0;

    printf("Choose tracking difficulty (1 for easy, 2 for hard): ");
    scanf("%d", &difficultyLevel);
    getchar(); 

    // Get player names
    printf("Enter name for Player 1: ");
    fgets(player1.name, MAX_NAME_LENGTH, stdin);
    player1.name[strcspn(player1.name, "\n")] = 0; 
    printf("Enter name for Player 2: ");
    fgets(player2.name, MAX_NAME_LENGTH, stdin);
    player2.name[strcspn(player2.name, "\n")] = 0; 

    // Define ships
    Ship ships[MAX_SHIPS] = {
        {"Carrier", 5, 0, 0},
        {"Battleship", 4, 0, 0},
        {"Destroyer", 3, 0, 0},
        {"Submarine", 2, 0, 0}
    };

    for (int i = 0; i < MAX_SHIPS; i++) {
        player1.ships[i] = ships[i];
        player2.ships[i] = ships[i];
    }

    // Initialize grids
    initializeGrid(&player1);
    initializeGrid(&player2);

    printf("%s goes first!\n", (turn == 0) ? player1.name : player2.name);

    // Ship placement for Player 1
    Player *currentPlayer = (turn == 0) ? &player1 : &player2;
    printf("%s, place your ships:\n", currentPlayer->name);
    for (int i = 0; i < MAX_SHIPS; i++) {
        char orientation;
        char coordinate[4];
        displayGrid((turn == 0) ? &player2 : &player1, currentPlayer, 1, 1); 
        if (currentPlayer->ships[i].size != 0) { 
            do {
                printf("Place %s (size %d): Enter coordinate (e.g., B3) and orientation (h/v): ", 
                        currentPlayer->ships[i].name, currentPlayer->ships[i].size);
                scanf("%s %c", coordinate, &orientation);
                if (orientation != 'h' && orientation != 'v') {
                    printf("Invalid orientation. Please enter 'h' for horizontal or 'v' for vertical.\n");
                }
            } while (orientation != 'h' && orientation != 'v');  

            if (placeShip(currentPlayer, i, coordinate, orientation) == 0) {
                i--; 
            }
            clearScreen(); 
        }
    }

    clearScreen();

    // Ship placement for Player 2
    currentPlayer = (turn == 1) ? &player1 : &player2; 
    printf("%s, place your ships:\n", currentPlayer->name);
    for (int i = 0; i < MAX_SHIPS; i++) {
        char orientation;
        char coordinate[4];
        displayGrid((turn == 1) ? &player1 : &player2, currentPlayer, 1, 1); 
        if (currentPlayer->ships[i].size != 0) {
            do {
                printf("Place %s (size %d): Enter coordinate (e.g., B3) and orientation (h/v): ", 
                        currentPlayer->ships[i].name, currentPlayer->ships[i].size);
                scanf("%s %c", coordinate, &orientation);
                if (orientation != 'h' && orientation != 'v') {
                    printf("Invalid orientation. Please enter 'h' for horizontal or 'v' for vertical.\n");
                }
            } while (orientation != 'h' && orientation != 'v');  

            if (placeShip(currentPlayer, i, coordinate, orientation) == 0) {
                i--; 
            }
            clearScreen(); 
        }
    }

    clearScreen();

    // Gameplay loop
    while (1) {
        Player *attackingPlayer = (turn % 2 == 0) ? &player1 : &player2;
        Player *defendingPlayer = (turn % 2 == 0) ? &player2 : &player1;

        displayGrid(defendingPlayer, attackingPlayer, 0, difficultyLevel == 1); 
        printf("%s's turn. Enter command (Fire [coordinate], Radar [coordinate], or Smoke Screen [coordinate]): ", attackingPlayer->name);
        fgets(command, sizeof(command), stdin);
        command[strcspn(command, "\n")] = 0; 

        if (strncmp(command, "Fire", 4) == 0) {
            fire(attackingPlayer, defendingPlayer, command + 5);
            checkSunkShips(attackingPlayer, defendingPlayer);
        } else if (strncmp(command, "Radar", 5) == 0) {
            radarSweep(attackingPlayer, command + 6, defendingPlayer);
        } else if (strncmp(command, "Smoke Screen", 12) == 0) {
            SmokeScreen(attackingPlayer, command + 13, defendingPlayer);
        } else {
            printf("Invalid command.\n");
            continue;
        }

        // Check for game over
        int allShipsSunk = 1;
        for (int i = 0; i < MAX_SHIPS; i++) {
            if (defendingPlayer->ships[i].hits < defendingPlayer->ships[i].size) {
                allShipsSunk = 0;
                break;
            }
        }

        if (allShipsSunk) {
            printf("%s wins!\n", attackingPlayer->name);
            break;
        }

        turn++;
    }

    return 0;
}

void initializeGrid(Player *player) {
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            player->grid[i][j] = '~';
        }
    }
}

void displayGrid(Player *player, Player *opponent, int revealShips, int trackMisses) {
    printf("   A B C D E F G H I J\n");
    for (int i = 0; i < GRID_SIZE; i++) {
        printf("%2d ", i + 1); 
        for (int j = 0; j < GRID_SIZE; j++) {
            if (opponent->grid[i][j] == 'H') {
                printf("H ");
            } else if (opponent->grid[i][j] == 'M' && trackMisses) {
                printf("M ");
            } else if (revealShips && opponent->grid[i][j] != '~') {
                printf("%c ", opponent->grid[i][j]);
            } else {
                printf("~ ");
            }
        }
        printf("\n");
    }
}

int placeShip(Player *player, int shipIndex, char *coordinate, char orientation) {
    int row, col;
    col = coordinate[0] - 'A';
    row = (coordinate[1] - '0');

    // Handle double-digit row inputs like A10
    if (coordinate[2] != '\0' && coordinate[2] >= '0' && coordinate[2] <= '9') {
        row = (coordinate[1] - '0') * 10 + (coordinate[2] - '0') - 1; 
    } else {
        row -= 1; 
    }

    // Check for out-of-bounds
    if (row < 0 || row >= GRID_SIZE || col < 0 || col >= GRID_SIZE) {
        printf("Invalid coordinates: out of bounds. Try again.\n");
        return 0; 
    }

    // Check if the placement is valid
    if (isValidPlacement(player, shipIndex, row, col, orientation)) {
        for (int i = 0; i < player->ships[shipIndex].size; i++) {
            player->grid[row][col] = player->ships[shipIndex].name[0]; 
            if (orientation == 'h') {
                col++;
            } else {
                row++;
            }
        }
        player->ships[shipIndex].placed = 1; 
        return 1; 
    } else {
        printf("Invalid placement. Please enter a new coordinate (e.g., B3) and orientation (h/v): ");
        return 0;
    }
}

int isValidPlacement(Player *player, int shipIndex, int row, int col, char orientation) {
    if (row < 0 || col < 0 || row >= GRID_SIZE || col >= GRID_SIZE) {
        printf("Placement out of bounds. Try again.\n");
        return 0; 
    }

    if (orientation == 'h') {
        if (col + player->ships[shipIndex].size > GRID_SIZE) {
            printf("Ship extends beyond the grid horizontally. Try again.\n");
            return 0; 
        }
        for (int i = 0; i < player->ships[shipIndex].size; i++) {
            if (player->grid[row][col + i] != '~') {
                printf("Overlap with another ship detected. Try again.\n");
                return 0; 
            }
        }
    } else { 
        if (row + player->ships[shipIndex].size > GRID_SIZE) {
            printf("Ship extends beyond the grid vertically. Try again.\n");
            return 0; 
        }
        for (int i = 0; i < player->ships[shipIndex].size; i++) {
            if (player->grid[row + i][col] != '~') {
                printf("Overlap with another ship detected. Try again.\n");
                return 0; 
            }
        }
    }
    return 1; 
}

void fire(Player *attacker, Player *defender, char *coordinate) {
    int row, col;
    col = coordinate[0] - 'A';

    if (coordinate[2] != '\0' && coordinate[2] >= '0' && coordinate[2] <= '9') {
        row = (coordinate[1] - '0') * 10 + (coordinate[2] - '0') - 1;
    } else {
        row = coordinate[1] - '1';
    }

    if (row < 0 || row >= GRID_SIZE || col < 0 || col >= GRID_SIZE) {
        printf("Invalid coordinates: out of bounds.\n");
        return;
    }

    if (defender->grid[row][col] != '~' && defender->grid[row][col] != 'H') {
        printf("Hit!\n");
        char shipChar = defender->grid[row][col];
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
    int row, col;
    col = coordinate[0] - 'A';
    row = (coordinate[1] - '0');

    if (coordinate[2] != '\0' && coordinate[2] >= '0' && coordinate[2] <= '9') {
        row = (coordinate[1] - '0') * 10 + (coordinate[2] - '0') - 1; 
    } else {
        row -= 1; 
    }

    if (row < 0 || row >= GRID_SIZE || col < 0 || col >= GRID_SIZE) {
        printf("Invalid coordinates for radar sweep.\n");
        return;
    }

    if (player->radarSweeps >= 3) {
        printf("%s has already used 3 radar sweeps and loses their turn.\n", player->name);
        return;
    }

    printf("Radar sweep at %s...\n", coordinate);
    int shipFound = 0;
    for (int i = row; i <= row + 1; i++) {
        for (int j = col; j <= col + 1; j++) {
            if (i >= 0 && i < GRID_SIZE && j >= 0 && j < GRID_SIZE) {
                if (opponent->grid[i][j] != '~' && opponent->grid[i][j] != 'O') {
                    shipFound = 1;
                    break;
                }
            }
        }
        if (shipFound) break;
    }

    if (shipFound) {
        printf("Enemy ships found!\n");
    } else {
        printf("No enemy ships found.\n");
    }

    player->radarSweeps++;
}

void SmokeScreen(Player *player, char *coordinate, Player *opponent) {
    int row, col;
    col = coordinate[0] - 'A';
    row = (coordinate[1] - '0');

    if (coordinate[2] != '\0' && coordinate[2] >= '0' && coordinate[2] <= '9') {
        row = (coordinate[1] - '0') * 10 + (coordinate[2] - '0') - 1; 
    } else {
        row -= 1; 
    }

    if (row < 0 || row >= GRID_SIZE - 1 || col < 0 || col >= GRID_SIZE - 1) {
        printf("Invalid coordinate input. Please choose a valid top-left coordinate for the area.\n");
        return;
    }

    if (player->SmokeScreen >= player->ShipsSunk) {
        printf("You are not allowed to use SmokeScreen. You lost your turn!\n");
        return;
    }

    player->SmokeScreen++;
    clearScreen(); 

    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            player->grid[row + i][col + j] = 'O'; 
        }
    }

    printf("Smoke Screen is applied at %s by %s.\n", coordinate, player->name);
}

void checkSunkShips(Player *player, Player *opponent) {
    for (int i = 0; i < MAX_SHIPS; i++) {
        if (opponent->ships[i].hits == opponent->ships[i].size && !opponent->ships[i].isSunk) {
            opponent->ships[i].isSunk = 1;  
            printf("Ship sunk: %s\n", opponent->ships[i].name);  
            player->ShipsSunk++;  
            return;  
        }
    }
}

int getRandomPlayer() {
    srand(time(NULL));
    return rand() % 2; 
}

void clearScreen() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}
