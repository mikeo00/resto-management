#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Basic structure for a menu item
typedef struct {
    int id;
    char name[256];
    char description[512];
    float price;
} MenuItem;

// Dynamic array to store menu items
typedef struct {
    MenuItem* items;
    int size;
    int capacity;
} Menu;

// Structure for order items (linked list)
typedef struct OrderItem {
    MenuItem* item;
    struct OrderItem* next;
} OrderItem;

// Structure for orders
typedef struct {
    int id;
    char customerName[256];
    OrderItem* items;
    bool completed;
    float total;
} Order;

// Node structure for orders in queue/stack
typedef struct Node {
    Order* order;
    struct Node* next;
} Node;

// Queue for active orders
typedef struct {
    Node* front;
    Node* rear;
    int size;
} OrderQueue;

// Stack for completed orders
typedef struct {
    Node* top;
    int size;
} OrderStack;

// Global variables
Menu menu;
OrderQueue activeOrders;
OrderStack completedOrders;
int nextOrderId = 1;

// Initialize the menu with initial capacity
void initMenu() {
    menu.items = malloc(10 * sizeof(MenuItem));
    menu.size = 0;
    menu.capacity = 10;
}

// Add a new menu item
void addMenuItem(int id, const char* name, const char* description, float price) {
    // Check for duplicate ID
    for (int i = 0; i < menu.size; i++) {
        if (menu.items[i].id == id) {
            printf("Item with ID %d already exists.\n", id);
            return;
        }
    }

    // Resize if needed
    if (menu.size >= menu.capacity) {
        menu.capacity *= 2;
        menu.items = realloc(menu.items, menu.capacity * sizeof(MenuItem));
    }

    // Add the new item
    menu.items[menu.size].id = id;
    strncpy(menu.items[menu.size].name, name, 255);
    strncpy(menu.items[menu.size].description, description, 511);
    menu.items[menu.size].price = price;
    menu.size++;
}

// Remove a menu item
void removeMenuItem(int id) {
    int index = -1;

    // Find the item
    for (int i = 0; i < menu.size; i++) {
        if (menu.items[i].id == id) {
            index = i;
            break;
        }
    }

    if (index == -1) {
        printf("Item with ID %d not found.\n", id);
        return;
    }

    // Shift remaining elements
    for (int i = index; i < menu.size - 1; i++) {
        menu.items[i] = menu.items[i + 1];
    }

    menu.size--;
    printf("Item with ID %d removed.\n", id);
}

// Find a menu item by ID
MenuItem* findMenuItem(int id) {
    for (int i = 0; i < menu.size; i++) {
        if (menu.items[i].id == id) {
            return &menu.items[i];
        }
    }
    return NULL;
}

// Clear the menu
void resetMenu() {
    menu.size = 0;
    printf("Menu has been reset.\n");
}

// Initialize order queue
void initOrderQueue() {
    activeOrders.front = NULL;
    activeOrders.rear = NULL;
    activeOrders.size = 0;
}

// Check if queue is empty
bool isQueueEmpty() {
    return activeOrders.front == NULL;
}

// Add an order to the queue
void enqueueOrder(Order* order) {
    Node* newNode = malloc(sizeof(Node));
    if (!newNode) {
        printf("Memory allocation failed\n");
        exit(1);
    }

    newNode->order = order;
    newNode->next = NULL;

    if (isQueueEmpty()) {
        activeOrders.front = newNode;
        activeOrders.rear = newNode;
    } else {
        activeOrders.rear->next = newNode;
        activeOrders.rear = newNode;
    }

    activeOrders.size++;
}

// Remove and return the first order from the queue
Order* dequeueOrder() {
    if (isQueueEmpty()) {
        return NULL;
    }

    Node* temp = activeOrders.front;
    Order* order = temp->order;

    activeOrders.front = activeOrders.front->next;
    if (activeOrders.front == NULL) {
        activeOrders.rear = NULL;
    }

    free(temp);
    activeOrders.size--;

    return order;
}

// Initialize order stack
void initOrderStack() {
    completedOrders.top = NULL;
    completedOrders.size = 0;
}

// Check if stack is empty
bool isStackEmpty() {
    return completedOrders.top == NULL;
}

// Push an order onto the stack
void pushOrder(Order* order) {
    Node* newNode = malloc(sizeof(Node));
    if (!newNode) {
        printf("Memory allocation failed\n");
        exit(1);
    }

    newNode->order = order;
    newNode->next = completedOrders.top;
    completedOrders.top = newNode;
    completedOrders.size++;
}

// Find and remove an order from queue by ID
void cancelOrder(int id) {
    if (isQueueEmpty()) {
        printf("No active orders to cancel.\n");
        return;
    }

    Node* current = activeOrders.front;
    Node* prev = NULL;

    // If the first order is the target
    if (current && current->order->id == id) {
        activeOrders.front = current->next;

        // If this was the only node, update rear as well
        if (activeOrders.front == NULL) {
            activeOrders.rear = NULL;
        }

        // Free order items
        OrderItem* itemCurrent = current->order->items;
        while (itemCurrent) {
            OrderItem* temp = itemCurrent;
            itemCurrent = itemCurrent->next;
            free(temp);
        }

        free(current->order);
        free(current);
        activeOrders.size--;

        printf("Order with ID %d has been cancelled.\n", id);
        return;
    }

    // Search for the order in the rest of the queue
    while (current && current->order->id != id) {
        prev = current;
        current = current->next;
    }

    // If order was not found
    if (!current) {
        printf("Order with ID %d not found.\n", id);
        return;
    }

    // Update rear pointer if removing the last node
    if (current == activeOrders.rear) {
        activeOrders.rear = prev;
    }

    // Remove the node
    prev->next = current->next;

    // Free order items
    OrderItem* itemCurrent = current->order->items;
    while (itemCurrent) {
        OrderItem* temp = itemCurrent;
        itemCurrent = itemCurrent->next;
        free(temp);
    }

    free(current->order);
    free(current);
    activeOrders.size--;

    printf("Order with ID %d has been cancelled.\n", id);
}

// Load menu from file
void loadMenu(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Could not open file %s. Creating a new menu.\n", filename);
        return;
    }

    char line[1024];
    char *token;

    // Clear current menu
    resetMenu();

    while (fgets(line, sizeof(line), file)) {
        // Remove newline character
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
        }

        // Parse line using comma as delimiter
        token = strtok(line, ",");
        if (!token) continue;
        int id = atoi(token);

        token = strtok(NULL, ",");
        if (!token) continue;
        char name[256];
        strncpy(name, token, 255);
        name[255] = '\0';

        token = strtok(NULL, ",");
        if (!token) continue;
        char description[512];
        strncpy(description, token, 511);
        description[511] = '\0';

        token = strtok(NULL, ",");
        if (!token) continue;
        float price = atof(token);

        addMenuItem(id, name, description, price);
    }

    fclose(file);
    printf("Menu loaded from %s.\n", filename);
}

// Save menu to file
void saveMenu(const char* filename) {
    FILE* file = fopen(filename, "w");
    if (!file) {
        printf("Could not open file %s for writing.\n", filename);
        return;
    }

    for (int i = 0; i < menu.size; i++) {
        fprintf(file, "%d,%s,%s,%.2f\n",
            menu.items[i].id,
            menu.items[i].name,
            menu.items[i].description,
            menu.items[i].price);
    }

    fclose(file);
    printf("Menu saved to %s.\n", filename);
}

// Save completed orders to file
void saveOrders(const char* filename) {
    FILE* file = fopen(filename, "w");
    if (!file) {
        printf("Could not open file %s for writing.\n", filename);
        return;
    }

    // Temporary stack to restore orders after writing
    OrderStack tempStack = {NULL, 0};

    fprintf(file, "--- Completed Orders ---\n\n");

    // Process all completed orders
    while (!isStackEmpty()) {
        Order* order = completedOrders.top->order;
        completedOrders.top = completedOrders.top->next;
        completedOrders.size--;

        fprintf(file, "Order ID: %d\n", order->id);
        fprintf(file, "Customer: %s\n", order->customerName);
        fprintf(file, "Items:\n");

        OrderItem* item = order->items;
        while (item) {
            fprintf(file, "- %s ($%.2f)\n", item->item->name, item->item->price);
            item = item->next;
        }

        fprintf(file, "Total: $%.2f\n\n", order->total);

        // Save to temp stack
        Node* newNode = malloc(sizeof(Node));
        newNode->order = order;
        newNode->next = tempStack.top;
        tempStack.top = newNode;
        tempStack.size++;
    }

    // Restore from temp stack
    while (tempStack.top) {
        Node* temp = tempStack.top;
        tempStack.top = tempStack.top->next;

        temp->next = completedOrders.top;
        completedOrders.top = temp;
        completedOrders.size++;
    }

    fclose(file);
    printf("Orders saved to %s.\n", filename);
}

// Display the menu
void displayMenu() {
    printf("\n--- Menu Items ---\n");
    if (menu.size == 0) {
        printf("No menu items available.\n");
        return;
    }

    for (int i = 0; i < menu.size; i++) {
        printf("ID: %d, Name: %s, Price: $%.2f\n",
            menu.items[i].id,
            menu.items[i].name,
            menu.items[i].price);
    }
}

// Add a new menu item (interactive)
void addMenuItemInteractive() {
    int id;
    char name[256];
    char description[512];
    float price;

    printf("Enter menu item ID: ");
    scanf("%d", &id);
    getchar(); // Consume newline

    printf("Enter menu item name: ");
    fgets(name, sizeof(name), stdin);
    name[strcspn(name, "\n")] = 0; // Remove newline

    printf("Enter menu item description: ");
    fgets(description, sizeof(description), stdin);
    description[strcspn(description, "\n")] = 0; // Remove newline

    printf("Enter menu item price: ");
    scanf("%f", &price);
    getchar(); // Consume newline

    addMenuItem(id, name, description, price);
    printf("Menu item added successfully!\n");
}

// Create a new order
void createOrder() {
    char customerName[256];
    int itemId;
    OrderItem* head = NULL;
    OrderItem* tail = NULL;
    float total = 0.0;

    printf("Enter customer name: ");
    fgets(customerName, sizeof(customerName), stdin);
    customerName[strcspn(customerName, "\n")] = 0; // Remove newline

    printf("Enter item IDs (0 to finish):\n");

    while (1) {
        scanf("%d", &itemId);
        if (itemId == 0) break;

        MenuItem* item = findMenuItem(itemId);
        if (!item) {
            printf("Item with ID %d not found. Try again.\n", itemId);
            continue;
        }

        // Create new order item
        OrderItem* newItem = malloc(sizeof(OrderItem));
        if (!newItem) {
            printf("Memory allocation failed\n");
            exit(1);
        }

        newItem->item = item;
        newItem->next = NULL;
        total += item->price;

        // Add to linked list
        if (!head) {
            head = newItem;
            tail = newItem;
        } else {
            tail->next = newItem;
            tail = newItem;
        }
    }
    getchar(); // Consume newline

    if (!head) {
        printf("No items selected. Order cancelled.\n");
        return;
    }

    // Create new order
    Order* newOrder = malloc(sizeof(Order));
    if (!newOrder) {
        printf("Memory allocation failed\n");
        exit(1);
    }

    newOrder->id = nextOrderId++;
    strncpy(newOrder->customerName, customerName, 255);
    newOrder->items = head;
    newOrder->completed = false;
    newOrder->total = total;

    // Add to queue
    enqueueOrder(newOrder);
    printf("Order #%d added successfully!\n", newOrder->id);
}

// Process the next order in queue
void processNextOrder() {
    if (isQueueEmpty()) {
        printf("No orders to process.\n");
        return;
    }

    Order* order = dequeueOrder();
    printf("Processing order #%d for %s...\n", order->id, order->customerName);

    order->completed = true;
    pushOrder(order);

    printf("Order processed successfully!\n");
}

// Display all orders
void displayOrders() {
    // Active orders
    if (isQueueEmpty()) {
        printf("\nNo active orders.\n");
    } else {
        printf("\n--- Active Orders ---\n");
        Node* current = activeOrders.front;

        while (current) {
            Order* order = current->order;
            printf("Order ID: %d, Customer: %s\n", order->id, order->customerName);
            printf("Items:\n");

            OrderItem* item = order->items;
            while (item) {
                printf("- %s ($%.2f)\n", item->item->name, item->item->price);
                item = item->next;
            }

            printf("Status: Pending\n\n");
            current = current->next;
        }
    }

    // Completed orders
    if (isStackEmpty()) {
        printf("No completed orders.\n");
    } else {
        printf("--- Completed Orders ---\n");
        Node* current = completedOrders.top;

        while (current) {
            Order* order = current->order;
            printf("Order ID: %d, Customer: %s\n", order->id, order->customerName);
            printf("Items:\n");

            OrderItem* item = order->items;
            while (item) {
                printf("- %s ($%.2f)\n", item->item->name, item->item->price);
                item = item->next;
            }

            printf("Status: Completed\n");
            printf("Total: $%.2f\n\n", order->total);
            current = current->next;
        }
    }
}

// Calculate total revenue
void calculateRevenue() {
    if (isStackEmpty()) {
        printf("No completed orders to calculate revenue.\n");
        return;
    }

    printf("\n--- Total Revenue ---\n");

    // Temporary stack to restore orders
    OrderStack tempStack = {NULL, 0};
    float totalRevenue = 0.0;

    // Process all completed orders
    while (!isStackEmpty()) {
        Order* order = completedOrders.top->order;
        completedOrders.top = completedOrders.top->next;
        completedOrders.size--;

        printf("Order %d: $%.2f\n", order->id, order->total);
        totalRevenue += order->total;

        // Save to temp stack
        Node* newNode = malloc(sizeof(Node));
        newNode->order = order;
        newNode->next = tempStack.top;
        tempStack.top = newNode;
        tempStack.size++;
    }

    printf("Total Revenue: $%.2f\n", totalRevenue);

    // Restore from temp stack
    while (tempStack.top) {
        Node* temp = tempStack.top;
        tempStack.top = tempStack.top->next;

        temp->next = completedOrders.top;
        completedOrders.top = temp;
        completedOrders.size++;
    }
}

// Clean up memory
void cleanup() {
    // Free menu
    free(menu.items);

    // Free active orders
    while (!isQueueEmpty()) {
        Order* order = dequeueOrder();

        // Free order items
        OrderItem* item = order->items;
        while (item) {
            OrderItem* temp = item;
            item = item->next;
            free(temp);
        }

        free(order);
    }

    // Free completed orders
    while (!isStackEmpty()) {
        Order* order = completedOrders.top->order;
        Node* temp = completedOrders.top;
        completedOrders.top = completedOrders.top->next;

        // Free order items
        OrderItem* item = order->items;
        while (item) {
            OrderItem* temp = item;
            item = item->next;
            free(temp);
        }

        free(order);
        free(temp);
    }
}

// Display main menu
void displayMainMenu() {
    printf("\n--- Restaurant Order Management System ---\n");
    printf("1. Display Menu\n");
    printf("2. Add Menu Item\n");
    printf("3. Delete Menu Item\n");
    printf("4. Reset Menu\n");
    printf("5. Add New Order\n");
    printf("6. Process Next Order\n");
    printf("7. Display Orders\n");
    printf("8. Cancel Order\n");
    printf("9. Calculate Total Revenue\n");
    printf("10. Save Orders to File\n");
    printf("11. Save Menu to File\n");
    printf("12. Exit\n");
    printf("Enter your choice: ");
}

int main() {
    // Initialize data structures
    initMenu();
    initOrderQueue();
    initOrderStack();

    // Load menu from file
    loadMenu("menu.txt");

    int choice;
    bool running = true;

    while (running) {
        displayMainMenu();
        scanf("%d", &choice);
        getchar(); // Consume newline

        switch (choice) {
            case 1:
                displayMenu();
                break;
            case 2:
                addMenuItemInteractive();
                break;
            case 3:
                printf("Enter the ID of the menu item to delete: ");
                int id;
                scanf("%d", &id);
                getchar(); // Consume newline
                removeMenuItem(id);
                break;
            case 4:
                printf("Are you sure you want to reset the menu? (y/n): ");
                char confirm;
                scanf(" %c", &confirm);
                getchar(); // Consume newline
                if (confirm == 'y' || confirm == 'Y') {
                    resetMenu();
                } else {
                    printf("Menu reset cancelled.\n");
                }
                break;
            case 5:
                createOrder();
                break;
            case 6:
                processNextOrder();
                break;
            case 7:
                displayOrders();
                break;
            case 8:
                printf("Enter the ID of the order to cancel: ");
                int orderId;
                scanf("%d", &orderId);
                getchar(); // Consume newline
                cancelOrder(orderId);
                break;
            case 9:
                calculateRevenue();
                break;
            case 10:
                saveOrders("completed_orders.txt");
                break;
            case 11:
                saveMenu("menu.txt");
                break;
            case 12:
                running = false;
                printf("Exiting the program... Goodbye!\n");
                break;
            default:
                printf("Invalid choice. Please try again.\n");
        }
    }

    // Clean up before exit
    cleanup();

    return 0;
}
