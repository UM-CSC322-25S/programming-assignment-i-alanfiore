#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_BOATS 120
#define MAX_NAME_LENGTH 128

/* Enumeration to represent different storage locations for boats */
typedef enum {
    slip,
    land,
    trailor,
    storage,
    no_place
} PlaceType;

/* Structure to hold boat details, including its location and outstanding charges */
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

/* Structure to manage an inventory of boats */
typedef struct {
    Boat *boats[MAX_BOATS];
    int boatCount;
} BoatManager;

/* Function to convert a string to the corresponding PlaceType enum */
PlaceType StringToPlaceType(char *PlaceString) {
    if (!strcasecmp(PlaceString, "slip")) return slip;
    if (!strcasecmp(PlaceString, "land")) return land;
    if (!strcasecmp(PlaceString, "trailor")) return trailor;
    if (!strcasecmp(PlaceString, "storage")) return storage;
    return no_place;
}

/* Function to convert a PlaceType enum to its corresponding string representation */
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

/* Function to read boat details from a CSV file and populate the BoatManager */
void ReadBoatsFromFile(BoatManager *manager, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file for reading");
        return;
    }

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

/* Function to write boat details to a CSV file */
void WriteBoatsToFile(BoatManager *manager, const char *filename) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        perror("Error opening file for writing");
        return;
    }
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

/* Function to add a boat using CSV input format */
void AddBoat(BoatManager *manager, char *csvData) {
    if (manager->boatCount >= MAX_BOATS) {
        printf("Error: Maximum number of boats reached.\n");
        return;
    }

    Boat *b = malloc(sizeof(Boat));
    if (!b) {
        printf("Error: Memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }

    char placeStr[16];
    char extra[16];
    int parsedFields = sscanf(csvData, "%127[^,],%d,%15[^,],%15[^,],%lf",
                               b->name, &b->length, placeStr, extra, &b->amountOwed);

    // Validate the number of fields parsed
    if (parsedFields != 5 || strlen(b->name) == 0 || b->length <= 0 || b->amountOwed < 0) {
        printf("Error: Invalid input format. Please enter data in the format:\n");
        printf("Name,Length,Place,Extra,AmountOwed\n");
        free(b);
        return;
    }

    b->place = StringToPlaceType(placeStr);
    switch (b->place) {
        case slip:
            b->location.slipNumber = atoi(extra);
            break;
        case land:
            if (strlen(extra) == 1 && isalpha(extra[0])) {
                b->location.bayLetter = extra[0];
            } else {
                printf("Error: Invalid bay letter for 'land'.\n");
                free(b);
                return;
            }
            break;
        case trailor:
            strncpy(b->location.trailerTag, extra, sizeof(b->location.trailerTag) - 1);
            b->location.trailerTag[sizeof(b->location.trailerTag) - 1] = '\0';
            break;
        case storage:
            b->location.storageNumber = atoi(extra);
            break;
        default:
            printf("Error: Invalid place type.\n");
            free(b);
            return;
    }

    // Add the boat to the inventory
    manager->boats[manager->boatCount++] = b;
}


/* Function to remove a boat by name */
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

/* Function to apply monthly storage charges based on boat type and length */
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

int CompareBoats(const void *a, const void *b) {
    // Cast `a` and `b` as pointers to `Boat`
    Boat *boatA = *(Boat **)a;
    Boat *boatB = *(Boat **)b;

    // Use `strcasecmp` for case-insensitive string comparison
    return strcasecmp(boatA->name, boatB->name);
}

void SortInventory(BoatManager *manager) {
    // Use `qsort` to sort boats alphabetically by name
    qsort(manager->boats, manager->boatCount, sizeof(Boat *), CompareBoats);
}

/* Main function to run the program */
int main(int argc, char *argv[]) {
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
        scanf(" %c", &choice);
        choice = tolower(choice);

        switch (choice) {
            case 'i': {
                // Display inventory
                SortInventory(&manager);
                for (int i = 0; i < manager.boatCount; i++) {
                    Boat *b = manager.boats[i];

                    // Print Boat Name, Length, and Place Type
                    printf("%-20s %4d'    %-8s", b->name, b->length, PlaceToString(b->place));

                    // Add Location-Specific Details
                    switch (b->place) {
                        case slip:
                            printf("  # %-6d", b->location.slipNumber); // Slip number formatted
                            break;
                        case land:
                            printf("     %-6c", b->location.bayLetter); // Bay letter formatted
                            break;
                        case trailor:
                            printf("  %-8s", b->location.trailerTag); // Trailer tag formatted
                            break;
                        case storage:
                            printf("  # %-6d", b->location.storageNumber); // Storage number formatted
                            break;
                        default:
                            printf("          "); // Add blank spaces for unknown place
                            break;
                    }
                    // Print Amount Owed
                    printf("   Owes $%7.2f\n", b->amountOwed); // Align and format monetary value
                }
                break;
            }


            case 'a': {
                // Add a new boat
                char csvData[256];
                printf("Enter boat data (CSV): ");
                getchar(); // Clear newline character
                fgets(csvData, sizeof(csvData), stdin);
                csvData[strcspn(csvData, "\n")] = 0; // Remove newline
                AddBoat(&manager, csvData);
                break;
            }
            case 'r': {
                // Remove a boat
                char name[MAX_NAME_LENGTH];
                printf("Enter boat name to remove: ");
                getchar();
                fgets(name, sizeof(name), stdin);
                name[strcspn(name, "\n")] = 0;
                RemoveBoat(&manager, name);
                break;
            }
            case 'p': {
                // Process a payment
                char name[MAX_NAME_LENGTH];
                double payment;
                printf("Enter boat name for payment: ");
                getchar(); //
                fgets(name, sizeof(name), stdin);
                name[strcspn(name, "\n")] = 0; // Remove newline

                int found = 0;
                for (int i = 0; i < manager.boatCount; i++) {
                    if (!strcasecmp(manager.boats[i]->name, name)) {
                        found = 1;
                        printf("Enter payment amount: ");
                        scanf("%lf", &payment);
                        if (payment > manager.boats[i]->amountOwed) {
                            printf("That is more than the amount owed, $%.2f. Payment rejected.\n", manager.boats[i]->amountOwed);
                        } else {
                            manager.boats[i]->amountOwed -= payment;
                            printf("Payment of $%.2f accepted.\n", payment);
                        }
                        break;
                    }
                }
                if (!found) {
                    printf("No boat with that name\n");
                }
                break;
            }
            case 'm': {
                // Apply monthly charges
                ApplyMonthlyCharges(&manager);
                printf("Monthly charges applied.\n");
                break;
            }
            case 'x': {
                // Exit and save data to file
                WriteBoatsToFile(&manager, argv[1]);
                printf("Exiting the Boat Management System...\n");
                break;
            }
            default:
                printf("Invalid option '%c'. Try again.\n", choice);
                break;
        }
    } while (choice != 'x');

    // Free allocated memory
    for (int i = 0; i < manager.boatCount; i++) {
        free(manager.boats[i]);
    }

    return 0;
}