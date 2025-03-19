#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_BOATS 120
#define MAX_NAME_LENGTH 128
#define FILE_NAME "BoatData.csv"

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

Boat *boats[MAX_BOATS];
int boatCount = 0;

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
void ReadBoatsFromFile(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) return;
    
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (boatCount >= MAX_BOATS) break;
        
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
        boats[boatCount++] = b;
    }
    fclose(file);
}

// Function to write boats to file
void WriteBoatsToFile(const char *filename) {
    FILE *file = fopen(filename, "w");
    if (!file) return;
    for (int i = 0; i < boatCount; i++) {
        Boat *b = boats[i];
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
void AddBoat(char *csvData) {
    if (boatCount >= MAX_BOATS) return;
    
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
    
    boats[boatCount++] = b;
}

// Function to remove a boat
void RemoveBoat(char *name) {
    for (int i = 0; i < boatCount; i++) {
        if (!strcasecmp(boats[i]->name, name)) {
            free(boats[i]);
            for (int j = i; j < boatCount - 1; j++) {
                boats[j] = boats[j + 1];
            }
            boatCount--;
            return;
        }
    }
    printf("No boat with that name\n");
}

// Function to apply monthly charges
void ApplyMonthlyCharges() {
    for (int i = 0; i < boatCount; i++) {
        switch (boats[i]->place) {
            case slip:
                boats[i]->amountOwed += boats[i]->length * 12.50;
                break;
            case land:
                boats[i]->amountOwed += boats[i]->length * 14.00;
                break;
            case trailor:
                boats[i]->amountOwed += boats[i]->length * 25.00;
                break;
            case storage:
                boats[i]->amountOwed += boats[i]->length * 11.20;
                break;
            default:
                break;
        }
    }
}

// Function to sort boats alphabetically
int CompareBoats(const void *a, const void *b) {
    Boat *boatA = *(Boat **)a;
    Boat *boatB = *(Boat **)b;
    return strcasecmp(boatA->name, boatB->name);
}

void SortInventory() { 
    qsort(boats, boatCount, sizeof(Boat *), CompareBoats); }

int main(int argc, char *argv[]) {
    // Check for the correct number of command-line arguments
    if (argc != 2) {
        printf("Usage: %s <BoatData.csv>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    // Initialize the system and load boats from the file
    ReadBoatsFromFile(argv[1]);
    printf("\nWelcome to the Boat Management System\n-------------------------------------");

    char choice;
    do {
        // Display menu
        printf("\n(I)nventory, (A)dd, (R)emove, (P)ayment, (M)onth, e(X)it: ");
        scanf(" %c", &choice);
        choice = tolower(choice);

        switch (choice) {
            case 'i':
                // Display inventory
                SortInventory();
                printf("\nBoat Inventory:\n");
                for (int i = 0; i < boatCount; i++) {
                    Boat *b = boats[i];
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
            case 'a': {
                // Add a new boat
                char csvData[256];
                printf("Please enter the boat data in CSV format: ");
                getchar(); // Clear newline character
                fgets(csvData, sizeof(csvData), stdin);
                csvData[strcspn(csvData, "\n")] = 0; // Remove newline
                AddBoat(csvData);
                break;
            }
            case 'r': {
                // Remove a boat
                char name[MAX_NAME_LENGTH];
                printf("Please enter the boat name: ");
                getchar(); // Clear newline character
                fgets(name, sizeof(name), stdin);
                name[strcspn(name, "\n")] = 0; // Remove newline
                RemoveBoat(name);
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
                for (int i = 0; i < boatCount; i++) {
                    if (!strcasecmp(boats[i]->name, name)) {
                        found = 1;
                        printf("Please enter the amount to be paid: ");
                        scanf("%lf", &payment);
                        if (payment > boats[i]->amountOwed) {
                            printf("That is more than the amount owed, $%.2f\n", boats[i]->amountOwed);
                        } else {
                            boats[i]->amountOwed -= payment;
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
                ApplyMonthlyCharges();
                break;
            case 'x':
                // Exit and save data to file
                WriteBoatsToFile(FILE_NAME);
                printf("Exiting the Boat Management System...\n");
                break;
            default:
                printf("Invalid option %c\n", choice);
                break;
        }
    } while (choice != 'x');

    // Free allocated memory
    for (int i = 0; i < boatCount; i++) {
        free(boats[i]);
    }
    return 0;
}