#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define GRID_SIZE 10
#define MAX_SHIPS 4
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
    int nextTurnHasArtillery;
    int hasUsedArtilleryThisTurn;
    int nextTurnHasTorpedo;// New field for torpedo availability
} Player;

// Function prototypes
void initializeGrid(Player *player);
void displayGrid(Player *player, Player *opponent, int revealShips, int trackMisses, int difficultyLevel);
int placeShip(Player *player, int shipIndex, char *coordinate, char orientation);
int isValidPlacement(Player *player, int shipIndex, int row, int col, char orientation);
void fire(Player *attacker, Player *defender, char *coordinate);
void radarSweep(Player *player, char *coordinate, Player *opponent);
void SmokeScreen(Player *player, char *coordinate, Player *opponent);
void checkSunkShips(Player *attacker, Player *defender);
void artillery(Player *attacker, Player *defender, char *coordinate);
void torpedo(Player *attacker, Player *defender, char *coordinate);// New function for Torpedo
int getRandomPlayer();
void clearScreen();
void delay(int seconds);

int main() {
    Player player1, player2;
    int turn = getRandomPlayer();
    char command[50];
    int difficultyLevel;

    player1.radarSweeps = 0;
    player2.radarSweeps = 0;
    player1.nextTurnHasTorpedo = 0;  // Initialize torpedo availability
    player2.nextTurnHasTorpedo = 0;  // Initialize torpedo availability

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
    Player *currentPlayer = (turn == 0) ? &player1 : &player2; //chosing whos the current player
    printf("%s, place your ships:\n", currentPlayer->name);
    for (int i = 0; i < MAX_SHIPS; i++) {
        char orientation;
        char coordinate[4];
        displayGrid((turn == 0) ? &player2 : &player1, currentPlayer, 1, 1,difficultyLevel);
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
        displayGrid((turn == 1) ? &player1 : &player2, currentPlayer, 1, 1,difficultyLevel); 
        if (currentPlayer->ships[i].size != 0) {
            do {
                printf("Place %s (size %d): Enter coordinate (e.g., B3) and orientation (h/v): ", currentPlayer->ships[i].name, currentPlayer->ships[i].size);
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

        displayGrid(defendingPlayer, attackingPlayer, 0, difficultyLevel == 1,difficultyLevel);
        printf("%s's turn. Enter command (Fire or Radar or Smoke Screen or Artillery or Torpedo)[coordinate]: ", attackingPlayer->name);
        fgets(command, sizeof(command), stdin);
        command[strcspn(command, "\n")] = 0;

        if (strncmp(command, "Fire", 4) == 0) {
            fire(attackingPlayer, defendingPlayer, command + 5);
            checkSunkShips(attackingPlayer, defendingPlayer);
        } else if (strncmp(command, "Radar", 5) == 0) {
            radarSweep(attackingPlayer, command + 6, defendingPlayer);
        } else if (strncmp(command, "Smoke Screen", 12) == 0) {
            SmokeScreen(attackingPlayer, command + 13, defendingPlayer);
        } else if (strncmp(command, "Artillery", 9) == 0) {
            artillery(attackingPlayer, defendingPlayer, command + 10);
        } else if (strncmp(command, "Torpedo", 7) == 0) {
            if (attackingPlayer->nextTurnHasTorpedo) {
                torpedo(attackingPlayer, defendingPlayer, command + 8);
                attackingPlayer->nextTurnHasTorpedo = 0; // Reset torpedo availability
            } else {
                printf("Torpedo not available.\n");
            }
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

void displayGrid(Player *player, Player *opponent, int revealShips, int trackMisses,int difficultyLevel ) {
    printf("   A B C D E F G H I J\n");
    for (int i = 0; i < GRID_SIZE; i++) {
        printf("%2d ", i + 1); 
        for (int j = 0; j < GRID_SIZE; j++) {
            if (opponent->grid[i][j] == '*') {
                printf("* ");
            } else if (opponent->grid[i][j] == 'o' && trackMisses && difficultyLevel == 1) {
                printf("o ");
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
        delay(1);
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
        delay(1);
        return 0;
    }
}

int isValidPlacement(Player *player, int shipIndex, int row, int col, char orientation) {
    if (row < 0 || col < 0 || row >= GRID_SIZE || col >= GRID_SIZE) {
        printf("Placement out of bounds. Try again.\n");
        delay(1);
        return 0;
    }

    if (orientation == 'h') {
        if (col + player->ships[shipIndex].size > GRID_SIZE) {
            printf("Ship extends beyond the grid horizontally. Try again.\n");
            delay(1);
            return 0; 
        }
        for (int i = 0; i < player->ships[shipIndex].size; i++) {
            if (player->grid[row][col + i] != '~') {
                printf("Overlap with another ship detected. Try again.\n");
                delay(1);
                return 0;
            }
        }
    } else { 
        if (row + player->ships[shipIndex].size > GRID_SIZE) {
            printf("Ship extends beyond the grid vertically. Try again.\n");
            delay(1);
            return 0;
        }
        for (int i = 0; i < player->ships[shipIndex].size; i++) {
            if (player->grid[row + i][col] != '~') {
                printf("Overlap with another ship detected. Try again.\n");
                delay(1);
                return 0;
            }
        }
    }
    return 1;
}

void fire(Player *attacker, Player *defender, char *coordinate) {
    int row, col;
    if (strlen(coordinate) == 2) {
        col = coordinate[0] - 'A';
        row = coordinate[1] - '1';
    } else if (strlen(coordinate) == 3 && coordinate[1] == '1' && coordinate[2] == '0') {
        col = coordinate[0] - 'A';
        row = 9; // Row 10
    } else {
        printf("Invalid coordinates. Missed!\n");
        return;
    }

    if (row < 0 || col < 0 || row >= GRID_SIZE || col >= GRID_SIZE) {
        printf("Invalid coordinates. Missed!\n");
        return;
    }

    if (defender->grid[row][col] != '~' && defender->grid[row][col] != 'o' && defender->grid[row][col] != '*') {
        printf("Hit!\n");
        char shipChar = defender->grid[row][col];
        defender->grid[row][col] = '*';
        for (int i = 0; i < MAX_SHIPS; i++) {
            if (defender->ships[i].name[0] == shipChar) {
                defender->ships[i].hits++;
                break;
            }
        }
    } else {
        printf("Miss!\n");
        defender->grid[row][col] = 'o';
    }
}

void radarSweep(Player *player, char *coordinate, Player *opponent) {
    int row, col;
    if (strlen(coordinate) == 2) {
        col = coordinate[0] - 'A';
        row = coordinate[1] - '1';
    } else if (strlen(coordinate) == 3 && coordinate[1] == '1' && coordinate[2] == '0') {
        col = coordinate[0] - 'A';
        row = 9; // Row 10
    } else {
        printf("Invalid coordinates for radar sweep.\n");
        return;
    }

    if (row < 0 || col < 0 || row >= GRID_SIZE || col >= GRID_SIZE) {
        printf("Invalid coordinates for radar sweep.\n");
        return;
    }

    if (player->radarSweeps >= 3) {
        printf("%s has already used 3 radar sweeps and loses their turn.\n", player->name);
        return;
    }

printf("Radar sweep at %s...\n", coordinate);
    int shipFound = 0;
    for (int i = row; i <= row + 1 && i < GRID_SIZE; i++) {
        for (int j = col; j <= col + 1 && j < GRID_SIZE; j++) {
            if (opponent->grid[i][j] == 'X') {
            continue;
            }
            if (opponent->grid[i][j] != '~' && opponent->grid[i][j] != 'o' &&  opponent->grid[i][j] != '*') {
                shipFound = 1; // Ship found
                break;
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
    if (player->SmokeScreen >= player->ShipsSunk) {
        printf("You are not allowed to use Smoke Screen. You lost your turn!\n");
        return;
    }

    int row, col;
    if (strlen(coordinate) == 2) {
        col = coordinate[0] - 'A';
        row = coordinate[1] - '1';
    } else if (strlen(coordinate) == 3 && coordinate[1] == '1' && coordinate[2] == '0') {
        col = coordinate[0] - 'A';
        row = 9; // Row 10
    } else {
        printf("Invalid coordinate input. Please choose a valid top-left coordinate for the area.\n");
        return;
    }
    // Ensure the 2x2 area is within bounds
    if (row < 0 || col < 0 || row + 1 >= GRID_SIZE || col + 1 >= GRID_SIZE) {
        printf("Invalid coordinate input. Please choose a valid top-left coordinate for the area.\n");
        return;
    }

    // Apply smoke screen
    for (int i = row; i <= row + 1; i++) {
        for (int j = col; j <= col + 1; j++) {
            opponent->grid[i][j] = 'X'; // Mark the area as obscured
        }
    }

    player->SmokeScreen++;
    clearScreen();
    printf("Smoke Screen is applied at %s by %s.\n", coordinate, player->name);
}

void checkSunkShips(Player *attacker, Player *defender) {
    int sunkShips = 0;
    for (int i = 0; i < MAX_SHIPS; i++) {
        if (defender->ships[i].hits == defender->ships[i].size && !defender->ships[i].isSunk) {
            defender->ships[i].isSunk = 1;
            printf("Ship sunk: %s\n", defender->ships[i].name);
            attacker->ShipsSunk++;
            sunkShips++;
            printf("number of ship sunked: %d\n",sunkShips);
        }
    }

    // Unlock torpedo when the third ship is sunk
    if (sunkShips >= 3) {
        attacker->nextTurnHasTorpedo = 1;
        printf("%s has unlocked the Torpedo for the next turn!\n", attacker->name);
    }
}

void artillery(Player *attacker, Player *defender, char *coordinate) {
    if (!attacker->nextTurnHasArtillery) {
        printf("Artillery is not available for this turn!\n");
        return;
    }

    int row, col;
    if (strlen(coordinate) == 2) {
        col = coordinate[0] - 'A';
        row = coordinate[1] - '1';
    } else if (strlen(coordinate) == 3 && coordinate[1] == '1' && coordinate[2] == '0') {
        col = coordinate[0] - 'A';
        row = 9; // Row 10
    } else {
        printf("Invalid coordinates for artillery strike.\n");
        return;
    }

    // Ensure the 2x2 grid is within bounds
    if (row < 0 || col < 0 || row + 1 >= GRID_SIZE || col + 1 >= GRID_SIZE) {
        printf("Invalid coordinates for artillery strike.\n");
        return;
    }

    printf("Artillery strike at %s...\n", coordinate);

    // Loop through the 2x2 grid and fire at each coordinate
    for (int i = row; i <= row + 1; i++) {
        for (int j = col; j <= col + 1; j++) {
            char targetCoordinate[3];
            targetCoordinate[0] = 'A' + j;    // Convert column index to letter
            targetCoordinate[1] = '1' + i;    // Convert row index to number
            targetCoordinate[2] = '\0';       // Null-terminate the string

            // Use fire method for each cell in the 2x2 area
            fire(attacker, defender, targetCoordinate);
        }
    }

    // Reset artillery availability for the next turn (it's only allowed once)
    attacker->nextTurnHasArtillery = 0;
}

void torpedo(Player *attacker, Player *defender, char *coordinate) {
    int isColumn = (coordinate[0] >= 'A' && coordinate[0] <= 'J');
    int row = -1, col = -1;

    if (isColumn) {
        col = coordinate[0] - 'A';
        printf("Torpedo fired at column %c...\n", coordinate[0]);
        for (int i = 0; i < GRID_SIZE; i++) {
            if (defender->grid[i][col] != '~' && defender->grid[i][col] != 'o' && defender->grid[i][col] != '*') {
                printf("Hit at %c%d!\n", 'A' + col, i + 1);
                defender->grid[i][col] = '*';
            }
        }
    } else {
        row = coordinate[0] - '1';
        printf("Torpedo fired at row %d...\n", row + 1);
        for (int j = 0; j < GRID_SIZE; j++) {
            if (defender->grid[row][j] != '~' && defender->grid[row][j] != 'o' && defender->grid[row][j] != '*') {
                printf("Hit at %c%d!\n", 'A' + j, row + 1);
                defender->grid[row][j] = '*';
            }
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

void delay(int seconds) {
    int milli_seconds = 1000 * seconds;
    clock_t start_time = clock();
    while (clock() < start_time + milli_seconds);
}
