#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_BOATS 120
#define MAX_NAME_LENGTH 128

typedef enum {
    slip,
    land,
    trailor,
    storage,
    no_place
} PlaceType;

typedef struct {
    char name[MAX_NAME_LENGTH];
    int length;
    PlaceType place;
    union {
        int slipNumber;
        char bayLetter;
        char trailerTag[16];
        int storageNumber;
    } location;
    double amountOwed;
} Boat;

typedef struct {
    Boat *boats[MAX_BOATS];
    int boatCount;
} BoatManager;

// Function to convert string to PlaceType
PlaceType StringToPlaceType(char *PlaceString) {
    if (!strcasecmp(PlaceString, "slip")) return slip;
    if (!strcasecmp(PlaceString, "land")) return land;
    if (!strcasecmp(PlaceString, "trailor")) return trailor;
    if (!strcasecmp(PlaceString, "storage")) return storage;
    return no_place;
}

// Function to convert PlaceType to string
char *PlaceToString(PlaceType Place) {
    switch (Place) {
        case slip: return "slip";
        case land: return "land";
        case trailor: return "trailor";
        case storage: return "storage";
        case no_place: return "no_place";
        default: exit(EXIT_FAILURE);
    }
}

// Function to read boats from file
void ReadBoatsFromFile(BoatManager *manager, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) return;
    
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (manager->boatCount >= MAX_BOATS) break;

        Boat *b = malloc(sizeof(Boat));
        if (!b) exit(EXIT_FAILURE);

        char placeStr[16];
        char extra[16];
        sscanf(line, "%127[^,],%d,%15[^,],%15[^,],%lf", b->name, &b->length, placeStr, extra, &b->amountOwed);

        b->place = StringToPlaceType(placeStr);
        switch (b->place) {
            case slip: b->location.slipNumber = atoi(extra); break;
            case land: b->location.bayLetter = extra[0]; break;
            case trailor: strncpy(b->location.trailerTag, extra, 15); break;
            case storage: b->location.storageNumber = atoi(extra); break;
            default: break;
        }
        manager->boats[manager->boatCount++] = b;
    }
    fclose(file);
}

// Function to write boats to file
void WriteBoatsToFile(BoatManager *manager, const char *filename) {
    FILE *file = fopen(filename, "w");
    if (!file) return;
    for (int i = 0; i < manager->boatCount; i++) {
        Boat *b = manager->boats[i];
        fprintf(file, "%s,%d,%s,", b->name, b->length, PlaceToString(b->place));
        switch (b->place) {
            case slip: fprintf(file, "%d", b->location.slipNumber); break;
            case land: fprintf(file, "%c", b->location.bayLetter); break;
            case trailor: fprintf(file, "%s", b->location.trailerTag); break;
            case storage: fprintf(file, "%d", b->location.storageNumber); break;
            default: break;
        }
        fprintf(file, ",%.2f\n", b->amountOwed);
    }
    fclose(file);
}

// Function to add a boat
void AddBoat(BoatManager *manager, char *csvData) {
    if (manager->boatCount >= MAX_BOATS) return;

    Boat *b = malloc(sizeof(Boat));
    if (!b) exit(EXIT_FAILURE);

    char placeStr[16];
    char extra[16];
    sscanf(csvData, "%127[^,],%d,%15[^,],%15[^,],%lf", b->name, &b->length, placeStr, extra, &b->amountOwed);

    b->place = StringToPlaceType(placeStr);
    switch (b->place) {
        case slip: b->location.slipNumber = atoi(extra); break;
        case land: b->location.bayLetter = extra[0]; break;
        case trailor: strncpy(b->location.trailerTag, extra, 15); break;
        case storage: b->location.storageNumber = atoi(extra); break;
        default: break;
    }

    manager->boats[manager->boatCount++] = b;
}

// Function to remove a boat
void RemoveBoat(BoatManager *manager, char *name) {
    for (int i = 0; i < manager->boatCount; i++) {
        if (!strcasecmp(manager->boats[i]->name, name)) {
            free(manager->boats[i]);
            for (int j = i; j < manager->boatCount - 1; j++) {
                manager->boats[j] = manager->boats[j + 1];
            }
            manager->boatCount--;
            return;
        }
    }
    printf("No boat with that name\n");
}

// Function to apply monthly charges
void ApplyMonthlyCharges(BoatManager *manager) {
    for (int i = 0; i < manager->boatCount; i++) {
        switch (manager->boats[i]->place) {
            case slip:
                manager->boats[i]->amountOwed += manager->boats[i]->length * 12.50;
                break;
            case land:
                manager->boats[i]->amountOwed += manager->boats[i]->length * 14.00;
                break;
            case trailor:
                manager->boats[i]->amountOwed += manager->boats[i]->length * 25.00;
                break;
            case storage:
                manager->boats[i]->amountOwed += manager->boats[i]->length * 11.20;
                break;
            default:
                break;
        }
    }
}

// Function to sort boats alphabetically
void SortInventory(BoatManager *manager) {
    for (int i = 0; i < manager->boatCount - 1; i++) {
        for (int j = 0; j < manager->boatCount - i - 1; j++) {
            if (strcasecmp(manager->boats[j]->name, manager->boats[j + 1]->name) > 0) {
                Boat *temp = manager->boats[j];
                manager->boats[j] = manager->boats[j + 1];
                manager->boats[j + 1] = temp;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    // Check for the correct number of command-line arguments
    if (argc != 2) {
        printf("Usage: %s <BoatData.csv>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Validate the file
    FILE *file = fopen(argv[1], "r");
    if (!file) {
        printf("Error: Unable to open file '%s'. Please ensure the file exists and try again.\n", argv[1]);
        exit(EXIT_FAILURE);
    }
    fclose(file);

    // Initialize BoatManager
    BoatManager manager = { .boatCount = 0 };

    // Load boats from file
    ReadBoatsFromFile(&manager, argv[1]);
    printf("\nWelcome to the Boat Management System\n-------------------------------------");

    char choice;
    do {
        // Display menu
        printf("\n(I)nventory, (A)dd, (R)emove, (P)ayment, (M)onth, e(X)it: ");
        if (scanf(" %c", &choice) != 1) { 
            // Handle invalid input that is not a char
            printf("Invalid input. Please enter a valid option.\n");
            // Clear the input buffer
            while (getchar() != '\n');
            continue;
        }

        choice = tolower(choice);
        switch (choice) {
            case 'i': {
                // Display inventory
                SortInventory(&manager);
                printf("\nBoat Inventory:\n");
                for (int i = 0; i < manager.boatCount; i++) {
                    Boat *b = manager.boats[i];
                    printf("%-20s %3d' %-8s", b->name, b->length, PlaceToString(b->place));
                    switch (b->place) {
                        case slip: printf("  # %2d", b->location.slipNumber); break;
                        case land: printf("      %c", b->location.bayLetter); break;
                        case trailor: printf("  %s", b->location.trailerTag); break;
                        case storage: printf("   # %2d", b->location.storageNumber); break;
                        default: break;
                    }
                    printf("   Owes $%.2f\n", b->amountOwed);
                }
                break;
            }
            case 'a': {
                // Add a new boat
                char csvData[256];
                printf("Please enter the boat data in CSV format: ");
                getchar(); // Clear newline character
                fgets(csvData, sizeof(csvData), stdin);
                csvData[strcspn(csvData, "\n")] = 0; // Remove newline
                AddBoat(&manager, csvData);
                break;
            }
            case 'r': {
                // Remove a boat
                char name[MAX_NAME_LENGTH];
                printf("Please enter the boat name: ");
                getchar(); // Clear newline character
                fgets(name, sizeof(name), stdin);
                name[strcspn(name, "\n")] = 0; // Remove newline
                RemoveBoat(&manager, name);
                break;
            }
            case 'p': {
                // Process a payment
                char name[MAX_NAME_LENGTH];
                double payment;
                printf("Please enter the boat name: ");
                getchar(); // Clear newline character
                fgets(name, sizeof(name), stdin);
                name[strcspn(name, "\n")] = 0; // Remove newline

                int found = 0;
                for (int i = 0; i < manager.boatCount; i++) {
                    if (!strcasecmp(manager.boats[i]->name, name)) {
                        found = 1;
                        printf("Please enter the amount to be paid: ");
                        if (scanf("%lf", &payment) != 1) {
                            printf("Invalid input. Please enter a valid number for payment.\n");
                            while (getchar() != '\n'); // Clear buffer
                            break;
                        }
                        if (payment > manager.boats[i]->amountOwed) {
                            printf("That is more than the amount owed, $%.2f\n", manager.boats[i]->amountOwed);
                        } else {
                            manager.boats[i]->amountOwed -= payment;
                        }
                        break;
                    }
                }
                if (!found) {
                    printf("No boat with that name\n");
                }
                break;
            }
            case 'm':
                // Apply monthly charges
                ApplyMonthlyCharges(&manager);
                break;
            case 'x':
                // Exit and save data to file
                WriteBoatsToFile(&manager, argv[1]);
                printf("Exiting the Boat Management System...\n");
                break;
            default:
                printf("Invalid option '%c'. Please try again.\n", choice);
                break;
        }
    } while (choice != 'x');

    // Free allocated memory
    for (int i = 0; i < manager.boatCount; i++) {
        free(manager.boats[i]);
    }

    return 0;
}
