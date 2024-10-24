#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define GRID_SIZE 10
#define MAX_SHIPS 5
#define MAX_NAME_LENGTH 50
// Added MaxSmokes for the smoke screen method (Sara)
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
    int radarSweeps; // rama: Counter for radar sweeps, because each player is allowed to use radar 3 times in the whole game so it keeps track of that
    int SmokeScreen;// to check how many times the player used the smoke creen method (Sara)
    int ShipsSunk; // the count of ships sunk by the player(Sara)
    // i need to check it after finishing SmokeScreen method. Checking where I need to update it whenever the player sunk opponent's ship(Sara)
    // Sara: check how we know that the player sunk a ship for the opponent.
} Player;

// Function prototypes
void initializeGrid(Player *player);
void displayGrid(Player *player, Player *opponent, int revealShips, int trackMisses);
int placeShip(Player *player, int shipIndex, char *coordinate, char orientation);
int isValidPlacement(Player *player, int shipIndex, int row, int col, char orientation);
void fire(Player *attacker, Player *defender, char *coordinate);
void radarSweep(Player *player, char *coordinate, Player *opponent);
// Here I put the prototype of SmokeScreen (Sara)
void SmokeScreen(Player *player, char *coordinate, Player *opponent);
void checkSunkShips(Player *attacker, Player *defender);
int getRandomPlayer();
void clearScreen();

int main() {
    Player player1, player2;
    int turn = getRandomPlayer(); // Randomly select starting player
    char command[50];
    int difficultyLevel;

// rama : to make sure it starts with zero before using the radar and then it increments
    player1.radarSweeps = 0;
    player2.radarSweeps = 0;

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

    // Inform players of who goes first
    printf("%s goes first!\n", (turn == 0) ? player1.name : player2.name);

   // Ship placement for the first player
Player *currentPlayer = (turn == 0) ? &player1 : &player2;
printf("%s, place your ships:\n", currentPlayer->name);
for (int i = 0; i < MAX_SHIPS; i++) {
    char orientation;
    char coordinate[3];
    displayGrid((turn == 0) ? &player2 : &player1, currentPlayer, 1, 1); // Show opponent's grid
    if(currentPlayer->ships[i].size != 0){  // Check that the ship size is not 0
        do {
            printf("Place %s (size %d): Enter coordinate (e.g., B3) and orientation (h/v): ", currentPlayer->ships[i].name, currentPlayer->ships[i].size);
            scanf("%s %c", coordinate, &orientation);
            if (orientation != 'h' && orientation != 'v') {
                printf("Invalid orientation. Please enter 'h' for horizontal or 'v' for vertical.\n");
            }
        } while (orientation != 'h' && orientation != 'v');  // Loop until a valid orientation is entered

        if (placeShip(currentPlayer, i, coordinate, orientation) == 0) {
            i--; // Repeat for invalid placement
        }
        clearScreen(); // Clear screen after each ship placement
    }
}

    // Clear screen after Player 1 finishes placing ships
    clearScreen();

    // Ship placement for the second player
    currentPlayer = (turn == 1) ? &player1 : &player2; // Switch to the other player
    printf("%s, place your ships:\n", currentPlayer->name);
    for (int i = 0; i < MAX_SHIPS; i++) {
        char orientation;
        char coordinate[3];
        displayGrid((turn == 1) ? &player1 : &player2, currentPlayer, 1, 1); // Show opponent's grid
        if(currentPlayer->ships[i].size!=0){
        printf("Place %s (size %d): Enter coordinate (e.g., B3) and orientation (h/v): ", currentPlayer->ships[i].name, currentPlayer->ships[i].size);
        scanf("%s %c", coordinate, &orientation);}
        if (placeShip(currentPlayer, i, coordinate, orientation) == 0) {
            i--; // Repeat for invalid placement
        }
        clearScreen(); // Clear screen after each ship placement
    }

    // Clear screen after Player 2 finishes placing ships
    clearScreen();

    // Gameplay loop
    //Sara: not sure about the methods call here. attention for the comment of Smokescreen
    while (1) {
        Player *attackingPlayer = (turn % 2 == 0) ? &player1 : &player2;
        Player *defendingPlayer = (turn % 2 == 0) ? &player2 : &player1;

        displayGrid(defendingPlayer, attackingPlayer, 0, difficultyLevel == 1); // Show opponent's grid, track misses if easy
        printf("%s's turn. Enter command (Fire [coordinate] or Radar [coordinate]): ", attackingPlayer->name);
        fgets(command, sizeof(command), stdin);
        command[strcspn(command, "\n")] = 0; // Remove newline

        if (strncmp(command, "Fire", 4) == 0) {
            fire(attackingPlayer, defendingPlayer, command + 5);
            checkSunkShips(attackingPlayer,defendingPlayer);
        } else if (strncmp(command, "Radar", 5) == 0) {
            radarSweep(attackingPlayer, command + 6, defendingPlayer);
        }else if(strncmp(command, "Smoke Screen",12)==0){
            SmokeScreen(attackingPlayer,command+13, defendingPlayer);
            // we need now to look when the opponent player call radar sweep after the player used smoke screen to say they are misses

        }
        
        else {
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
                printf("%c ", opponent->grid[i][j]); // Show opponent's ships
            } else {
                printf("~ "); // Water
            }
        }
        printf("\n");
    }
}
int placeShip(Player *player, int shipIndex, char *coordinate, char orientation) {
    int row, col;

    while (1) {
        row = coordinate[1] - '1';   // Parse row (1-based to 0-based)
        col = coordinate[0] - 'A';   // Parse column (A-J to 0-9)

        // Check if the placement is valid
        if (isValidPlacement(player, shipIndex, row, col, orientation)) {
            // Place the ship on the grid
            for (int i = 0; i < player->ships[shipIndex].size; i++) {
                player->grid[row][col] = player->ships[shipIndex].name[0]; // Update grid with ship's character
                if (orientation == 'h') {
                    col++;
                } else {
                    row++;
                }
            }
            player->ships[shipIndex].placed = 1; // Mark as placed
            return 1; // Successful placement
        } else {
            // Inform the user about the specific issue
            printf("Invalid placement. Please enter a new coordinate (e.g., B3) and orientation (h/v): ");
            scanf("%s %c", coordinate, &orientation);
        }
    }
}
int isValidPlacement(Player *player, int shipIndex, int row, int col, char orientation) {
    // Check if the starting point is out of bounds
    if (row < 0 || col < 0 || row >= GRID_SIZE || col >= GRID_SIZE) {
        printf("Placement out of bounds. Try again.\n");
        return 0; // Out of bounds
    }

    // Check if the ship will fit within the grid
    if (orientation == 'h') {
        if (col + player->ships[shipIndex].size > GRID_SIZE) {
            printf("Ship extends beyond the grid horizontally. Try again.\n");
            return 0; // Out of bounds horizontally
        }
        // Check for overlaps
        for (int i = 0; i < player->ships[shipIndex].size; i++) {
            if (player->grid[row][col + i] != '~') {
                printf("Overlap with another ship detected. Try again.\n");
                return 0; // Space occupied
            }
        }
    } else { // Vertical placement
        if (row + player->ships[shipIndex].size > GRID_SIZE) {
            printf("Ship extends beyond the grid vertically. Try again.\n");
            return 0; // Out of bounds vertically
        }
        // Check for overlaps
        for (int i = 0; i < player->ships[shipIndex].size; i++) {
            if (player->grid[row + i][col] != '~') {
                printf("Overlap with another ship detected. Try again.\n");
                return 0; // Space occupied
            }
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
//RAMA : this will check wether or not the ship exists in the 2*2 area (found or not) without showing the exact location
// and there's a comdition : max 3 times in the whole game if did more -> loses turn
void radarSweep(Player *player, char *coordinate, Player *opponent) {
    int row = coordinate[1] - '1';
    int col = coordinate[0] - 'A';
    if (row < 0 || col < 0 || row >= GRID_SIZE || col >= GRID_SIZE) {
        printf("Invalid coordinates for radar sweep.\n");
        return;
    }
    if (player->radarSweeps >= 3) {
        printf("%s has already used 3 radar sweeps and loses their turn.\n", player->name);
        return;
    }
    else{
        printf("Radar sweep at %s...\n", coordinate);
    // Reveal the grid of the opponent for that area
    int shipFound=0;
    for (int i = row ; i <= row + 1; i++) {
        for (int j = col ; j <= col + 1; j++) {
            if (i >= 0 && i < GRID_SIZE && j >= 0 && j < GRID_SIZE) {
                if (opponent->grid[i][j] != '~' || opponent->grid[i][j] != 'O' ) {
                    shipFound=1; // to check if found
                    break;
                }
            }
        }
        if(shipFound) break; // exit the loop
    }
    if (shipFound) {
        printf("Enemy ships found!\n");
    } else {
        printf("No enemy ships found.\n");
    }

    // Increment the radar sweep counter
    player->radarSweeps++;
    return;}
}
// here I added the Smoke Screem (Sara)
void SmokeScreen(Player *player, char *coordinate, Player *opponent){
    if(player->SmokeScreen >= player-> ShipsSunk){
        printf("You are not allowed to use SmokeScreen. You lost your turn!");
        return;
    }
    int col =coordinate[0]-'A';
    int row=coordinate[1]-'1';
    //-2 to ensure that the 2x2 area specified doesn't go out of the bound of the grid
    if(row<0 ||row>GRID_SIZE-2 || col<0|| col>GRID_SIZE-2){
        // not sure if the player put wrong coordinate, they lose their turn?
        printf("Invalid coordinate input. Please choose a valid topleft coordinatefor the area.\n");
        return;

    }
    player->SmokeScreen++;//Sara
    clearScreen(); // Sara: we use it to preserve secrecy
    //making the chosen area obscured
    for(int i=0;i<2;i++){
        for(int j=0;j<2;j++){
            player->grid[row+i][col+j]='O';// obscured
        }
    }// Sara I'm not sure if we shouldn't print this statement
    printf("Smoke Screen is applied at %s by %s.\n", coordinate,player->name);
    

}
//Sara: printing the ship that's sunk after each move
void checkSunkShips(Player *player, Player *opponent) {
    // Loop through each of the opponent's ships to check if they are sunk
    for (int i = 0; i < MAX_SHIPS; i++) {
        // Check if the ship is fully hit (sunk) and hasn't been marked as sunk yet
        if (opponent->ships[i].hits == opponent->ships[i].size && !opponent->ships[i].isSunk) {
            opponent->ships[i].isSunk = 1;  // Mark the ship as sunk
            printf("Ship sunk: %s\n", opponent->ships[i].name);  // Print the name of the ship that was sunk
            player->ShipsSunk++;  // Increment player's sunk ships count
            return;  // Exit the function after printing the sunk ship to avoid multiple prints
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
