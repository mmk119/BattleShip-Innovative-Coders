#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define GRID_SIZE 10
#define MAX_SHIPS 4
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
void displayGrid(Player *player, Player *opponent, int revealShips);
int placeShip(Player *player, int shipIndex, int row, int col, char orientation);
int isValidPlacement(Player *player, int shipIndex, int row, int col, char orientation);
void fire(Player *attacker, Player *defender, char *coordinate);
void radarSweep(Player *player, char *coordinate, Player *opponent);
int getRandomPlayer();
void clearScreen();

int main() {
    Player player1, player2;
    int turn = getRandomPlayer();
    char command[50];

    // Initialize players
    strcpy(player1.name, "Player 1");
    strcpy(player2.name, "Player 2");

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

    // Ship placement
    printf("%s, place your ships:\n", player1.name);
    for (int i = 0; i < MAX_SHIPS; i++) {
        char orientation;
        int row, col;
        displayGrid(&player1, &player2, 1);
        printf("Place %s (size %d): Enter row, column and orientation (h/v): ", player1.ships[i].name, player1.ships[i].size);
        scanf("%d %d %c", &row, &col, &orientation);
        if (placeShip(&player1, i, row - 1, col - 1, orientation) == 0) {
            i--; // Repeat for invalid placement
        }
        clearScreen();
    }

    printf("%s, place your ships:\n", player2.name);
    for (int i = 0; i < MAX_SHIPS; i++) {
        char orientation;
        int row, col;
        displayGrid(&player2, &player1, 1);
        printf("Place %s (size %d): Enter row, column and orientation (h/v): ", player2.ships[i].name, player2.ships[i].size);
        scanf("%d %d %c", &row, &col, &orientation);
        if (placeShip(&player2, i, row - 1, col - 1, orientation) == 0) {
            i--; // Repeat for invalid placement
        }
        clearScreen();
    }

    // Gameplay loop
    while (1) {
        Player *currentPlayer = (turn % 2 == 0) ? &player1 : &player2;
        Player *opponentPlayer = (turn % 2 == 0) ? &player2 : &player1;

        displayGrid(currentPlayer, opponentPlayer, 0);
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
            player->grid[i][j] = '~';
        }
    }
}

void displayGrid(Player *player, Player *opponent, int revealShips) {
    printf("  A B C D E F G H I J\n");
    for (int i = 0; i < GRID_SIZE; i++) {
        printf("%d ", i + 1);
        for (int j = 0; j < GRID_SIZE; j++) {
            if (opponent->grid[i][j] == 'H') {
                printf("H ");
            } else if (opponent->grid[i][j] == 'M') {
                printf("M ");
            } else if (revealShips && opponent->grid[i][j] != '~') {
                printf("S ");
            } else {
                printf("~ ");
            }
        }
        printf(" | ");
        for (int j = 0; j < GRID_SIZE; j++) {
            printf("%c ", player->grid[i][j]);
        }
        printf("\n");
    }
}

int placeShip(Player *player, int shipIndex, int row, int col, char orientation) {
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
    }
    printf("Invalid placement. Try again.\n");
    return 0;
}

int isValidPlacement(Player *player, int shipIndex, int row, int col, char orientation) {
    if (row < 0 || col < 0 || row >= GRID_SIZE || col >= GRID_SIZE) return 0;
    if (orientation == 'h') {
        if (col + player->ships[shipIndex].size > GRID_SIZE) return 0;
        for (int i = 0; i < player->ships[shipIndex].size; i++) {
            if (player->grid[row][col + i] != '~') return 0;
        }
    } else {
        if (row + player->ships[shipIndex].size > GRID_SIZE) return 0;
        for (int i = 0; i < player->ships[shipIndex].size; i++) {
            if (player->grid[row + i][col] != '~') return 0;
        }
    }
    return 1;
}

void fire(Player *attacker, Player *defender, char *coordinate) {
    int row = coordinate[1] - '1';
    int col = coordinate[0] - 'A';
    if (row < 0 || col < 0 || row >= GRID_SIZE || col >= GRID_SIZE) {
        printf("Invalid coordinates. Missed!\n");
        return;
    }

    if (defender->grid[row][col] != '~') {
        printf("Hit!\n");
        defender->grid[row][col] = 'H';
        // Update ship hits
        for (int i = 0; i < MAX_SHIPS; i++) {
            if (defender->grid[row][col] == defender->ships[i].name[0]) {
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
    if (row < 0 || col < 0 || row + 1 >= GRID_SIZE || col + 1 >= GRID_SIZE) {
        printf("Invalid radar sweep area.\n");
        return;
    }

    int found = 0;
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            if (opponent->grid[row + i][col + j] != '~') {
                found = 1;
            }
        }
    }

    if (found) {
        printf("Enemy ships found!\n");
    } else {
        printf("No enemy ships found.\n");
    }
}

int getRandomPlayer() {
    srand(time(NULL));
    return rand() % 2;
}

void clearScreen() {
    system("clear || cls");
}