#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

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
    int isBot;
} Player;
char smoke_grid[GRID_SIZE][GRID_SIZE];   // rama : Auxiliary grid to mark smoke-covered cells

// Function prototypes
void initializeGrid(Player *player);
void displayGrid(Player *player, Player *opponent, int revealShips, int trackMisses, int difficultyLevel);
int placeShip(Player *player, int shipIndex, char *coordinate, char orientation, int isBot);
int isValidPlacement(Player *player, int shipIndex, int row, int col, char orientation,int isBot);
void fire(Player *attacker, Player *defender, char *coordinate);
void radarSweep(Player *player, char *coordinate, Player *opponent);
void SmokeScreen(Player *player, char *coordinate, Player *opponent);
void checkSunkShips(Player *attacker, Player *defender);
void artillery(Player *attacker, Player *defender, char *coordinate);
void torpedo(Player *attacker, Player *defender, char *coordinate);// New function for Torpedo
int getRandomPlayer();
void clearScreen();
void delay(int seconds);
void activate_smoke_screen(int x, int y); // rama 
void toLowerCase(char *str);
void toUpperCase(char *str);
void generateRandomPlacement(char *coordinate, char *orientation);
void placeShipsRandomly(Player *currentPlayer);
void BotFire( Player *defender, int row, int col);
void botEasy(Player *bot, Player *opponent);
void botMedium(Player *bot, Player *opponent);
void botHard(Player *bot, Player *opponent);

int main() {
    Player player1, player2;
    int turn = getRandomPlayer();
    char command[50];
    int difficultyLevel;
    int choice;
    player1.radarSweeps = 0;
    player2.radarSweeps = 0;
    player1.nextTurnHasTorpedo = 0;  // Initialize torpedo availability
    player2.nextTurnHasTorpedo = 0;  // Initialize torpedo availability


    char input[10];  // Buffer for user input

    while (1) {
        printf("Choose tracking difficulty (1 for easy, 2 for medium, 3 for hard): ");
        fgets(input, sizeof(input), stdin); 

        
        input[strcspn(input, "\n")] = 0;

        char *endptr;
        difficultyLevel = strtol(input, &endptr, 10); // Convert to integer

       
        if (*endptr != '\0' || difficultyLevel < 1 || difficultyLevel > 3) {
            printf("Invalid Input. Please choose a correct value (1, 2, or 3).\n");
        } else {
            break;
        }
    }

        while (1) {
        printf("Press 1 for single player or 2 for multiplayers: ");
        fgets(input, sizeof(input), stdin); 

        
        input[strcspn(input, "\n")] = 0;

        char *endptr;
        difficultyLevel = strtol(input, &endptr, 10); // Convert to integer

       
        if (*endptr != '\0' || difficultyLevel < 1 || difficultyLevel > 2) {
            printf("Invalid Input. Please choose a correct value (1, or 2).\n");
        } else {
            break;
        }
    }

    if (choice == 2){
        // Get player names
        printf("Enter name for Player 1: ");
        fgets(player1.name, MAX_NAME_LENGTH, stdin);
        player1.name[strcspn(player1.name, "\n")] = 0;
        player1.isBot = 0;
        printf("Enter name for Player 2: ");
        fgets(player2.name, MAX_NAME_LENGTH, stdin);
        player2.name[strcspn(player2.name, "\n")] = 0;
        player2.isBot = 0;
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
                    toLowerCase(&orientation);
                    toUpperCase(coordinate);
                    printf(coordinate);
                    if (orientation != 'h' && orientation != 'v') {
                        printf("Invalid orientation. Please enter 'h' for horizontal or 'v' for vertical.\n");
                    }
                } while (orientation != 'h' && orientation != 'v');  

                if (placeShip(currentPlayer, i, coordinate, orientation,0) == 0) {
                    i--; 
                }
                clearScreen();
            }
        }

        clearScreen();

        // Ship placement for bot
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
                    toLowerCase(&orientation);
                    toUpperCase(coordinate);
                    if (orientation != 'h' && orientation != 'v') {
                        printf("Invalid orientation. Please enter 'h' for horizontal or 'v' for vertical.\n");
                    }
                } while (orientation != 'h' && orientation != 'v');

                if (placeShip(currentPlayer, i, coordinate, orientation,0) == 0) {
                    i--;
                }
                clearScreen();
            }
        }

        clearScreen();

            // After ship placement and before the while loop
            int c;
            while ((c = getchar()) != '\n' && c != EOF) {}
        int first = 0;
        // Gameplay loop
        while (1) {
            Player *attackingPlayer = (turn % 2 == 0) ? &player1 : &player2;
            Player *defendingPlayer = (turn % 2 == 0) ? &player2 : &player1;
            if (first >= 1){
                displayGrid(defendingPlayer, attackingPlayer, 0, difficultyLevel == 1,difficultyLevel);
            }
           
            printf("%s's turn. Enter command (Fire or Radar or Smoke Screen or Artillery or Torpedo)[coordinate]: ", attackingPlayer->name);
            fgets(command, sizeof(command), stdin);
            command[strcspn(command, "\n")] = 0;

            if ( (strncmp(command, "Fire", 4) == 0)  || (strncmp(command, "fire", 4) == 0)) {
                fire(attackingPlayer, defendingPlayer, command + 5);
                checkSunkShips(attackingPlayer, defendingPlayer);
            } else if ( (strncmp(command, "Radar", 5) == 0)  || (strncmp(command, "radar", 5) == 0) ){
                radarSweep(attackingPlayer, command + 6, defendingPlayer);
            } else if ( (strncmp(command, "Smoke Screen", 12) == 0) || (strncmp(command, "smoke screen", 12) == 0) ){
                SmokeScreen(attackingPlayer, command + 13, defendingPlayer);
            } else if ( (strncmp(command, "Artillery", 9) == 0) || (strncmp(command, "artillery", 9) == 0) ) {
                artillery(attackingPlayer, defendingPlayer, command + 10);
            } else if ( (strncmp(command, "Torpedo", 7) == 0) || (strncmp(command, "torpedo", 7) == 0) ) {
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
            first +=1;
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
    // Single player mode with bots
    else{
        printf("Enter name for the player: ");
        fgets(player1.name, MAX_NAME_LENGTH, stdin);
        player1.name[strcspn(player1.name, "\n")] = 0;
        player1.isBot = 0;

        strcpy(player2.name, "Bot Player");
        player2.isBot = 1;
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

        Player *currentPlayer = &player1; 
        printf("%s, place your ships:\n", currentPlayer->name);
        for (int i = 0; i < MAX_SHIPS; i++) {
            char orientation;
            char coordinate[4];
            displayGrid(&player1, currentPlayer, 1, 1,difficultyLevel);
            if (currentPlayer->ships[i].size != 0) { 
                do {
                    printf("Place %s (size %d): Enter coordinate (e.g., B3) and orientation (h/v): ", 
                            currentPlayer->ships[i].name, currentPlayer->ships[i].size);
                    scanf("%s %c", coordinate, &orientation);
                    toLowerCase(&orientation);
                    toUpperCase(coordinate);
                    printf(coordinate);
                    if (orientation != 'h' && orientation != 'v') {
                        printf("Invalid orientation. Please enter 'h' for horizontal or 'v' for vertical.\n");
                    }
                } while (orientation != 'h' && orientation != 'v');  

                if (placeShip(currentPlayer, i, coordinate, orientation,0) == 0) {
                    i--; 
                }
                clearScreen();
            }
        }

        clearScreen();
        
        // placing ships for the bot in a random way
        placeShipsRandomly(&player2);
        displayGrid(&player2, &player2, 1, 1,difficultyLevel);

            // After ship placement and before the while loop
            int c;
            while ((c = getchar()) != '\n' && c != EOF) {}
        int first = 0;
        // Gameplay loop
        while (1) {
            Player *attackingPlayer = (turn % 2 == 0) ? &player1 : &player2;
            Player *defendingPlayer = (turn % 2 == 0) ? &player2 : &player1;
           
            if (! attackingPlayer->isBot){
                printf("%s's turn. Enter command (Fire or Radar or Smoke Screen or Artillery or Torpedo)[coordinate]: ", attackingPlayer->name);
                fgets(command, sizeof(command), stdin);
                command[strcspn(command, "\n")] = 0;
                
                if ( (strncmp(command, "Fire", 4) == 0)  || (strncmp(command, "fire", 4) == 0)) {
                    fire(attackingPlayer, defendingPlayer, command + 5);
                    checkSunkShips(attackingPlayer, defendingPlayer);
                } else if ( (strncmp(command, "Radar", 5) == 0)  || (strncmp(command, "radar", 5) == 0) ){
                    radarSweep(attackingPlayer, command + 6, defendingPlayer);
                } else if ( (strncmp(command, "Smoke Screen", 12) == 0) || (strncmp(command, "smoke screen", 12) == 0) ){
                    SmokeScreen(attackingPlayer, command + 13, defendingPlayer);
                } else if ( (strncmp(command, "Artillery", 9) == 0) || (strncmp(command, "artillery", 9) == 0) ) {
                    artillery(attackingPlayer, defendingPlayer, command + 10);
                } else if ( (strncmp(command, "Torpedo", 7) == 0) || (strncmp(command, "torpedo", 7) == 0) ) {
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
               
                displayGrid( attackingPlayer , defendingPlayer, 0, difficultyLevel == 1,difficultyLevel);
               
            }
            else{
                printf("Bot turn \n");

                // logic for bot play go here
                switch (difficultyLevel) {
                        case 1: botEasy(&player2, &player1); break;
                        case 2: botMedium(&player2, &player1); break;
                        case 3: botHard(&player2, &player1); break;
                        default: printf("Invalid difficulty level.\n"); exit(1);
                        
                    }
                
                displayGrid(attackingPlayer, defendingPlayer , 0, difficultyLevel == 1,difficultyLevel);
                    
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
}

void initializeGrid(Player *player) {
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            player->grid[i][j] = '~';
        }
    }
}

void displayGrid(Player *player, Player *opponent, int revealShips, int trackMisses,int difficultyLevel ) {
    printf("\n   A B C D E F G H I J\n");
    for (int i = 0; i < GRID_SIZE; i++) {
        printf("%2d ", i + 1); 
        for (int j = 0; j < GRID_SIZE; j++) {
            if (opponent->grid[i][j] == '*') {
                printf("* ");
            } else if (opponent->grid[i][j] == 'o') {
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
int placeShip(Player *player, int shipIndex, char *coordinate, char orientation, int isBot) {
    toUpperCase(coordinate);
    toLowerCase(&orientation);
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
        if (isBot != 1){
            printf("Invalid coordinates: out of bounds. Try again.\n");
            delay(1);
            
        }
        return 0;
    }

    // Check if the placement is valid
    if (isValidPlacement(player, shipIndex, row, col, orientation, isBot)) {
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
        if (isBot != 1){
            printf("Invalid placement. Please enter a new coordinate (e.g., B3) and orientation (h/v): ");
            delay(1);
        }
        return 0;
    }
}

int isValidPlacement(Player *player, int shipIndex, int row, int col, char orientation, int isBot) {
    toLowerCase(&orientation);
    if (row < 0 || col < 0 || row >= GRID_SIZE || col >= GRID_SIZE) {
        if (isBot != 1){
            printf("Placement out of bounds. Try again.\n");
            delay(1);
           
        }
         return 0;
    }

    if (orientation == 'h') {
        if (col + player->ships[shipIndex].size > GRID_SIZE) {
            if (isBot != 1){
                printf("Ship extends beyond the grid horizontally. Try again.\n");
                delay(1);
            
            }
                return 0; 
        }
        for (int i = 0; i < player->ships[shipIndex].size; i++) {
            if (player->grid[row][col + i] != '~') {
                if (isBot != 1){
                    printf("Overlap with another ship detected. Try again.\n");
                    delay(1);
                    
                }
                return 0;
            }
        }
    } else { 
        if (row + player->ships[shipIndex].size > GRID_SIZE) {
            if (isBot != 1){
                printf("Ship extends beyond the grid vertically. Try again.\n");
                delay(1);
               
            }
             return 0;
        }
        for (int i = 0; i < player->ships[shipIndex].size; i++) {
            if (player->grid[row + i][col] != '~') {
                if (isBot != 1){
                    printf(" Overlap with another ship detected. Try again.\n");
                    delay(1);
                    
                }
                return 0;
            }
        }
    }
    return 1;
}

void fire(Player *attacker, Player *defender, char *coordinate) {
    toUpperCase(coordinate);
    int row, col;
    if (strlen(coordinate) == 2) {
        col = coordinate[0] - 'A';
        row = coordinate[1] - '1';
    } else if (strlen(coordinate) == 3 && coordinate[1] == '1' && coordinate[2] == '0') {
        col = coordinate[0] - 'A';
        row = 9; // Row 10
    } else {
        printf("Invalid coordinates.\n");
        return;
    }

    if (row < 0 || col < 0 || row >= GRID_SIZE || col >= GRID_SIZE) {
        printf("Invalid coordinates.\n");
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
    } 
    else if(defender->grid[row][col] == '*'){
        printf("Ship already hit here!\n");
    }
    else {
        printf("Miss!\n");
        defender->grid[row][col] = 'o';
    }
}

// Radar sweep function with smoke screen check
void radarSweep(Player *player, char *coordinate, Player *opponent) {
  toUpperCase(coordinate);
  int row, col;
  if (strlen(coordinate) == 2) {
      col = coordinate[0] - 'A';
      row = coordinate[1] - '1';
  } else if (strlen(coordinate) == 3 && coordinate[1] == '1' && coordinate[2] == '0') {
      col = coordinate[0] - 'A';
      row = 9;
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

 
  int shipFound = 0;
  for (int i = row; i <= row + 1 && i < GRID_SIZE; i++) {
      for (int j = col; j <= col + 1 && j < GRID_SIZE; j++) {
          if (smoke_grid[i][j] == 'X') {
             printf("No enemy ships found.\n");
              return;  // Exit if smoke screen is present
          }
          if (opponent->grid[i][j] != '~' && opponent->grid[i][j] != 'o' && opponent->grid[i][j] != '*') {
              shipFound = 1;
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

// Smoke screen function with validation and applying smoke screen effect
void SmokeScreen(Player *player, char *coordinate, Player *opponent) {
  toUpperCase(coordinate);
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
      row = 9;
  } else {
      printf("Invalid coordinate input. Please choose a valid top-left coordinate for the area.\n");
      return;
  }
  
  if (row < 0 || col < 0 || row + 1 >= GRID_SIZE || col + 1 >= GRID_SIZE) {
      printf("Invalid coordinate input. Please choose a valid top-left coordinate for the area.\n");
      return;
  }

  // Call activate_smoke_screen to apply the smoke on the opponent's grid
  activate_smoke_screen(row, col);

  player->SmokeScreen++;
  
}
void activate_smoke_screen(int x, int y) {
  for (int i = x; i < x + 2 && i < GRID_SIZE; i++) {
      for (int j = y; j < y + 2 && j < GRID_SIZE; j++) {
          smoke_grid[i][j] = 'X';  // Mark as obscured in the smoke grid
      }
  }
}

void checkSunkShips(Player *attacker, Player *defender) {
    int totalSunkShips = 0;
    int allShipsSunk = 1;
    for (int i = 0; i < MAX_SHIPS; i++) {
        if (defender->ships[i].hits == defender->ships[i].size && !defender->ships[i].isSunk) {
            defender->ships[i].isSunk = 1;  // Mark the ship as sunk
            printf("Ship sunk: %s\n", defender->ships[i].name);
            attacker->ShipsSunk++;  // Increment attacker's sunk ships count
            totalSunkShips++;
        }
        // If there's at least one ship that isn't sunk, set allShipsSunk to 0
    if (!defender->ships[i].isSunk) {
            allShipsSunk = 0;
    }
    
        }
     // If all defender's ships are sunk, declare the winner and exit
    if (allShipsSunk) {
        return;
    }

    // If at least one ship is sunk during this turn, unlock artillery for the next turn
    if (totalSunkShips > 0) {
        attacker->nextTurnHasArtillery = 1;
        printf("%s has unlocked Artillery and Smoke Screen for the next turn!\n", attacker->name);
    }

    // Unlock torpedo when exactly three ships have been sunk
    if (attacker->ShipsSunk == 3) {
        attacker->nextTurnHasTorpedo = 1;
        printf("%s has unlocked the Torpedo for the next turn!\n", attacker->name);
    }
}


void artillery(Player *attacker, Player *defender, char *coordinate) {
    toUpperCase(coordinate);
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
    toUpperCase(coordinate);
    if (!attacker->nextTurnHasTorpedo) {
        printf("Torpedo is not available!\n");
        return;
    }

    int row = -1, col = -1;

    // Convert first character to uppercase to handle lowercase input
    coordinate[0] = toupper(coordinate[0]);

    // Validate and parse coordinate input for a single letter or digit
    if (strlen(coordinate) == 1) {
        if (isalpha(coordinate[0])) { // Case for column (A-J)
            col = coordinate[0] - 'A';
        } else if (isdigit(coordinate[0])) { // Case for row (1-9)
            row = coordinate[0] - '1';
        } else {
            printf("Invalid coordinate for torpedo strike.\n");
            return;
        }
    } else if (strlen(coordinate) == 2 && coordinate[0] == '1' && coordinate[1] == '0') { // Case for row 10
        row = 9;
    } else {
        printf("Invalid coordinate for torpedo strike.\n");
        return;
    }

    // Check if coordinates are within bounds
    if ((row < 0 && col < 0) || (row >= GRID_SIZE || col >= GRID_SIZE)) {
        printf("Invalid coordinates for torpedo strike.\n");
        return;
    }

    if (col != -1) { // Column-based torpedo strike
        printf("Torpedo strike at column %c...\n", coordinate[0]);
        for (int i = 0; i < GRID_SIZE; i++) {
            if (defender->grid[i][col] != '~' && defender->grid[i][col] != 'o' && defender->grid[i][col] != '*') {
                printf("Hit at %c%d!\n", coordinate[0], i + 1);
                char shipChar = defender->grid[i][col];
                defender->grid[i][col] = '*';
                for (int k = 0; k < MAX_SHIPS; k++) {
                    if (defender->ships[k].name[0] == shipChar) {
                        defender->ships[k].hits++;
                        break;
                    }
                }
            } else if (defender->grid[i][col] == '~') {
                defender->grid[i][col] = 'o';
            }
        }
    } else if (row != -1) { // Row-based torpedo strike
        printf("Torpedo strike at row %d...\n", row + 1);
        for (int j = 0; j < GRID_SIZE; j++) {
            if (defender->grid[row][j] != '~' && defender->grid[row][j] != 'o' && defender->grid[row][j] != '*') {
                printf("Hit at %c%d!\n", 'A' + j, row + 1);
                char shipChar = defender->grid[row][j];
                defender->grid[row][j] = '*';
                for (int k = 0; k < MAX_SHIPS; k++) {
                    if (defender->ships[k].name[0] == shipChar) {
                        defender->ships[k].hits++;
                        break;
                    }
                }
            } else if (defender->grid[row][j] == '~') {
                defender->grid[row][j] = 'o';
            }
        }
    }

    // Reset the torpedo flag after use
    attacker->nextTurnHasTorpedo = 0;
    printf("Torpedo has been used by %s!\n", attacker->name);
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

void toLowerCase(char *str) {
    char *temp = str; 

    while (*temp != '\0') {
        if ((*temp >= 'A') && (*temp <= 'Z')) {
            *temp += ('a' - 'A'); 
        }
        temp++;
    }
}
void toUpperCase(char *str) {
    char *temp = str; 

    while (*temp != '\0') {
        if ((*temp >= 'a') && (*temp <= 'z')) {
            *temp -= ('a' - 'A'); 
        }
        temp++; 
    }
}

void generateRandomPlacement(char *coordinate, char *orientation) {

    *orientation = (rand() % 2 == 0) ? 'h' : 'v';
    coordinate[0] = 'A' + (rand() % GRID_SIZE);
    int column = rand() % GRID_SIZE + 1;
    sprintf(&coordinate[1], "%d", column); 
    coordinate[3] = '\0'; 
}


void placeShipsRandomly(Player *currentPlayer) {
    printf("%s, placing ships randomly...\n", currentPlayer->name);
    for (int i = 0; i < MAX_SHIPS; i++) {
        char orientation;
        char coordinate[4];

        if (currentPlayer->ships[i].size != 0) {
            do {
                
                generateRandomPlacement(coordinate, &orientation);
                
                if (placeShip(currentPlayer, i, coordinate, orientation,1) == 0) {
                      i--; 
                    continue;
                }
            } while (0); 
        }
    }
}


// bot fire
void BotFire( Player *defender, int row, int col) {
    if (defender->grid[row][col] != '~' && defender->grid[row][col] != 'o' && defender->grid[row][col] != '*') {
        printf("Bot hits you!\n");
        char shipChar = defender->grid[row][col];
        defender->grid[row][col] = '*';
        for (int i = 0; i < MAX_SHIPS; i++) {
            if (defender->ships[i].name[0] == shipChar) {
                defender->ships[i].hits++;
                break;
            }
        }
    } 
    else {
        printf("Bot misses his shot!\n");
        defender->grid[row][col] = 'o';
    }
}


// Bot: Easy level
void botEasy(Player *bot, Player *opponent) {
    int row, col;
    do {
        row = rand() % GRID_SIZE;
        col = rand() % GRID_SIZE;
    } while (opponent->grid[row][col] == '*' || opponent->grid[row][col] == 'o');
    BotFire(opponent, row, col);
}

// Bot: Medium level
void botMedium(Player *bot, Player *opponent) {
    // Medium bot attacks intelligently based on previous hits
    static int lastHitRow = -1, lastHitCol = -1;
    if (lastHitRow != -1 && lastHitCol != -1) {
        int directions[4][2] = {{0, 1}, {1, 0}, {0, -1}, {-1, 0}};
        for (int i = 0; i < 4; i++) {
            int newRow = lastHitRow + directions[i][0];
            int newCol = lastHitCol + directions[i][1];
            if (newRow >= 0 && newRow < GRID_SIZE && newCol >= 0 && newCol < GRID_SIZE &&
                (opponent->grid[newRow ][newCol] != '*'  && opponent->grid[newRow][newCol] != 'o') ) {
                BotFire(opponent, newRow, newCol);
                lastHitRow = newRow;
                lastHitCol = newCol;
                return;
            }
        }
    }
    botEasy(bot, opponent);

    for (int row = 0; row < GRID_SIZE; row++) {
        for (int col = 0; col < GRID_SIZE; col++) {
            if (opponent->grid[row][col] == '*') {
                lastHitRow = row;
                lastHitCol = col;
                return;
            }
        }
    }

    // Reset to -1 if no hits exist
    lastHitRow = -1;
    lastHitCol = -1;
}

// Bot: Hard level
void botHard(Player *bot, Player *opponent) {
    int maxProb = 0, targetRow = 0, targetCol = 0;
    for (int row = 0; row < GRID_SIZE; row++) {
        for (int col = 0; col < GRID_SIZE; col++) {
            if (opponent->grid[row][col] == '~') {
                int prob = 0;
                for (int d = 1; d <= 5; d++) {
                    if (row + d < GRID_SIZE && (opponent->grid[row + d][col] == 'C'  || opponent->grid[row + d][col] == 'D' || opponent->grid[row + d][col] == 'B' || opponent->grid[row + d][col] == 'S' )) prob++;
                    if (col + d < GRID_SIZE && (opponent->grid[row + d][col] == 'C'  || opponent->grid[row + d][col] == 'D' || opponent->grid[row + d][col] == 'B' || opponent->grid[row + d][col] == 'S' )) prob++;
                }
                // printf("%d",prob);
                // printf("\n");
                if (prob > maxProb) {
                    maxProb = prob;
                    targetRow = row;
                    targetCol = col;
                }
            }
        }
    }
    BotFire(opponent, targetRow, targetCol);
}
