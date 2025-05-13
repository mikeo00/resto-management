

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_STRING 256
#define MAX_DESCRIPTION 512
#define INITIAL_CAPACITY 10
#define MENU_FILE "menu.txt"
#define ORDERS_FILE "completed_orders.txt"

typedef struct {
    int id;
    char name[MAX_STRING];
    char description[MAX_DESCRIPTION];
    float price;
} MenuItem;

typedef struct {
    MenuItem* items;
    int size;
    int capacity;
} DynamicArrayList;

typedef struct OrderItem {
    MenuItem* item;
    struct OrderItem* next;
} OrderItem;

// Order structure
typedef struct {
    int id;
    char customerName[MAX_STRING];
    OrderItem* items;
    bool completed;
    float total;
} Order;

// Node structure for Queue and Stack
typedef struct Node {
    Order* order;
    struct Node* next;
} Node;

// Queue structure for active orders
typedef struct {
    Node* front;
    Node* rear;
    int size;
} Queue;

// Stack structure for completed orders
typedef struct {
    Node* top;
    int size;
} Stack;

/* Global Variables */
DynamicArrayList menuList;
Queue activeOrders;
Stack completedOrders;
int orderIdCounter = 1;

/* Function Prototypes */

// DynamicArrayList operations
void initDynamicArrayList();
void addMenuItem(int id, const char* name, const char* description, float price);
void removeMenuItem(int id);
MenuItem* findMenuItem(int id);
void resetMenu();
void destroyDynamicArrayList();

// Queue operations
void initQueue();
void enqueue(Order* order);
Order* dequeue();
bool isQueueEmpty();
void displayQueue();
void destroyQueue();
Order* findOrderInQueue(int id);
void removeOrderFromQueue(int id);

// Stack operations
void initStack();
void push(Order* order);
Order* pop();
bool isStackEmpty();
void displayStack();
void destroyStack();

// File operations
bool loadMenuFromFile(const char* filename);
bool saveMenuToFile(const char* filename);
bool saveCompletedOrdersToFile(const char* filename);

// Application operations
void displayMenu();
void processAddMenuItem();
void processDeleteMenuItem();
void processResetMenu();
void processAddOrder();
void processNextOrder();
void displayOrders();
void processDeleteOrder();
void calculateTotalRevenue();
void displayMainMenu();

/* Implementation of DynamicArrayList operations */

void initDynamicArrayList() {
    menuList.items = (MenuItem*)malloc(INITIAL_CAPACITY * sizeof(MenuItem));
    if (menuList.items == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    menuList.size = 0;
    menuList.capacity = INITIAL_CAPACITY;
}

void addMenuItem(int id, const char* name, const char* description, float price) {
    // Check if id already exists
    for (int i = 0; i < menuList.size; i++) {
        if (menuList.items[i].id == id) {
            printf("Item with ID %d already exists. Choose a different ID.\n", id);
            return;
        }
    }

    // Resize if necessary
    if (menuList.size >= menuList.capacity) {
        menuList.capacity *= 2;
        MenuItem* temp = (MenuItem*)realloc(menuList.items, menuList.capacity * sizeof(MenuItem));
        if (temp == NULL) {
            fprintf(stderr, "Memory reallocation failed\n");
            exit(EXIT_FAILURE);
        }
        menuList.items = temp;
    }

    // Add the new item
    menuList.items[menuList.size].id = id;
    strncpy(menuList.items[menuList.size].name, name, MAX_STRING - 1);
    menuList.items[menuList.size].name[MAX_STRING - 1] = '\0';
    strncpy(menuList.items[menuList.size].description, description, MAX_DESCRIPTION - 1);
    menuList.items[menuList.size].description[MAX_DESCRIPTION - 1] = '\0';
    menuList.items[menuList.size].price = price;
    menuList.size++;
}

void removeMenuItem(int id) {
    int found = -1;

    // Find the item
    for (int i = 0; i < menuList.size; i++) {
        if (menuList.items[i].id == id) {
            found = i;
            break;
        }
    }

    if (found == -1) {
        printf("Item with ID %d not found.\n", id);
        return;
    }

    // Shift remaining elements
    for (int i = found; i < menuList.size - 1; i++) {
        menuList.items[i] = menuList.items[i + 1];
    }

    menuList.size--;
    printf("Item with ID %d removed successfully.\n", id);
}

MenuItem* findMenuItem(int id) {
    for (int i = 0; i < menuList.size; i++) {
        if (menuList.items[i].id == id) {
            return &menuList.items[i];
        }
    }
    return NULL;
}

void resetMenu() {
    menuList.size = 0;
    printf("Menu has been reset.\n");
}

void destroyDynamicArrayList() {
    free(menuList.items);
    menuList.items = NULL;
    menuList.size = 0;
    menuList.capacity = 0;
}

/* Implementation of Queue operations */

void initQueue() {
    activeOrders.front = NULL;
    activeOrders.rear = NULL;
    activeOrders.size = 0;
}

void enqueue(Order* order) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    if (newNode == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
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

Order* dequeue() {
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

bool isQueueEmpty() {
    return activeOrders.front == NULL;
}

Order* findOrderInQueue(int id) {
    Node* current = activeOrders.front;

    while (current != NULL) {
        if (current->order->id == id) {
            return current->order;
        }
        current = current->next;
    }

    return NULL;
}

void removeOrderFromQueue(int id) {
    if (isQueueEmpty()) {
        printf("No active orders to delete.\n");
        return;
    }

    Node* current = activeOrders.front;
    Node* prev = NULL;

    // If the first node is the target
    if (current != NULL && current->order->id == id) {
        activeOrders.front = current->next;

        // If this was the only node, update rear as well
        if (activeOrders.front == NULL) {
            activeOrders.rear = NULL;
        }

        // Free order items memory
        OrderItem* itemCurrent = current->order->items;
        while (itemCurrent != NULL) {
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
    while (current != NULL && current->order->id != id) {
        prev = current;
        current = current->next;
    }

    // If order was not found
    if (current == NULL) {
        printf("Order with ID %d not found.\n", id);
        return;
    }

    // Update the rear pointer if removing the last node
    if (current == activeOrders.rear) {
        activeOrders.rear = prev;
    }

    // Remove the node
    prev->next = current->next;

    // Free order items memory
    OrderItem* itemCurrent = current->order->items;
    while (itemCurrent != NULL) {
        OrderItem* temp = itemCurrent;
        itemCurrent = itemCurrent->next;
        free(temp);
    }

    free(current->order);
    free(current);
    activeOrders.size--;

    printf("Order with ID %d has been cancelled.\n", id);
}

void displayQueue() {
    if (isQueueEmpty()) {
        printf("No active orders.\n");
        return;
    }

    printf("\n--- Active Orders ---\n");
    Node* current = activeOrders.front;

    while (current != NULL) {
        Order* order = current->order;
        printf("Order ID: %d, Customer: %s\n", order->id, order->customerName);
        printf("Items:\n");

        OrderItem* itemCurrent = order->items;
        while (itemCurrent != NULL) {
            printf("- %s ($%.2f)\n", itemCurrent->item->name, itemCurrent->item->price);
            itemCurrent = itemCurrent->next;
        }

        printf("Status: Pending\n\n");
        current = current->next;
    }
}

void destroyQueue() {
    while (!isQueueEmpty()) {
        Order* order = dequeue();

        // Free order items
        OrderItem* current = order->items;
        while (current != NULL) {
            OrderItem* temp = current;
            current = current->next;
            free(temp);
        }

        free(order);
    }
}

/* Implementation of Stack operations */

void initStack() {
    completedOrders.top = NULL;
    completedOrders.size = 0;
}

void push(Order* order) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    if (newNode == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    newNode->order = order;
    newNode->next = completedOrders.top;
    completedOrders.top = newNode;
    completedOrders.size++;
}

Order* pop() {
    if (isStackEmpty()) {
        return NULL;
    }

    Node* temp = completedOrders.top;
    Order* order = temp->order;

    completedOrders.top = completedOrders.top->next;
    free(temp);
    completedOrders.size--;

    return order;
}

bool isStackEmpty() {
    return completedOrders.top == NULL;
}

void displayStack() {
    if (isStackEmpty()) {
        printf("No completed orders.\n");
        return;
    }

    printf("\n--- Completed Orders ---\n");
    Node* current = completedOrders.top;

    while (current != NULL) {
        Order* order = current->order;
        printf("Order ID: %d, Customer: %s\n", order->id, order->customerName);
        printf("Items:\n");

        OrderItem* itemCurrent = order->items;
        while (itemCurrent != NULL) {
            printf("- %s ($%.2f)\n", itemCurrent->item->name, itemCurrent->item->price);
            itemCurrent = itemCurrent->next;
        }

        printf("Status: Completed\n");
        printf("Total: $%.2f\n\n", order->total);
        current = current->next;
    }
}

void destroyStack() {
    while (!isStackEmpty()) {
        Order* order = pop();

        // Free order items
        OrderItem* current = order->items;
        while (current != NULL) {
            OrderItem* temp = current;
            current = current->next;
            free(temp);
        }

        free(order);
    }
}

/* Implementation of File operations */

bool loadMenuFromFile(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        printf("Could not open file %s. Creating a new menu.\n", filename);
        return false;
    }

    char line[1024];
    char *token;

    // Clear current menu
    resetMenu();

    while (fgets(line, sizeof(line), file)) {
        int id;
        char name[MAX_STRING];
        char description[MAX_DESCRIPTION];
        float price;

        // Remove newline character if present
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
        }

        // Parse line using comma as delimiter
        token = strtok(line, ",");
        if (token == NULL) continue;
        id = atoi(token);

        token = strtok(NULL, ",");
        if (token == NULL) continue;
        strncpy(name, token, MAX_STRING - 1);
        name[MAX_STRING - 1] = '\0';

        token = strtok(NULL, ",");
        if (token == NULL) continue;
        strncpy(description, token, MAX_DESCRIPTION - 1);
        description[MAX_DESCRIPTION - 1] = '\0';

        token = strtok(NULL, ",");
        if (token == NULL) continue;
        price = atof(token);

        addMenuItem(id, name, description, price);
    }

    fclose(file);
    printf("Menu loaded successfully from %s.\n", filename);
    return true;
}

bool saveMenuToFile(const char* filename) {
    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        printf("Could not open file %s for writing.\n", filename);
        return false;
    }

    for (int i = 0; i < menuList.size; i++) {
        fprintf(file, "%d,%s,%s,%.2f\n",
            menuList.items[i].id,
            menuList.items[i].name,
            menuList.items[i].description,
            menuList.items[i].price);
    }

    fclose(file);
    printf("Menu saved successfully to %s.\n", filename);
    return true;
}

bool saveCompletedOrdersToFile(const char* filename) {
    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        printf("Could not open file %s for writing.\n", filename);
        return false;
    }

    // Create a temp stack to restore completed orders after writing
    Stack tempStack;
    tempStack.top = NULL;
    tempStack.size = 0;

    fprintf(file, "--- Completed Orders ---\n\n");

    // Pop all items from completedOrders, write them to file, and push to tempStack
    while (!isStackEmpty()) {
        Order* order = pop();

        fprintf(file, "Order ID: %d\n", order->id);
        fprintf(file, "Customer: %s\n", order->customerName);
        fprintf(file, "Items:\n");

        OrderItem* itemCurrent = order->items;
        while (itemCurrent != NULL) {
            fprintf(file, "- %s ($%.2f)\n", itemCurrent->item->name, itemCurrent->item->price);
            itemCurrent = itemCurrent->next;
        }

        fprintf(file, "Total: $%.2f\n\n", order->total);

        // Push to temporary stack
        Node* newNode = (Node*)malloc(sizeof(Node));
        if (newNode == NULL) {
            fprintf(stderr, "Memory allocation failed\n");
            exit(EXIT_FAILURE);
        }

        newNode->order = order;
        newNode->next = tempStack.top;
        tempStack.top = newNode;
        tempStack.size++;
    }

    // Restore completed orders from tempStack
    while (tempStack.top != NULL) {
        Node* temp = tempStack.top;
        Order* order = temp->order;

        push(order);

        tempStack.top = tempStack.top->next;
        free(temp);
    }

    fclose(file);
    printf("Orders saved successfully to %s.\n", filename);
    return true;
}

/* Implementation of Application operations */

void displayMenu() {
    printf("\n--- Menu Items ---\n");
    if (menuList.size == 0) {
        printf("No menu items available.\n");
        return;
    }

    for (int i = 0; i < menuList.size; i++) {
        printf("ID: %d, Name: %s, Price: $%.2f\n",
            menuList.items[i].id,
            menuList.items[i].name,
            menuList.items[i].price);
    }
}

void processAddMenuItem() {
    int id;
    char name[MAX_STRING];
    char description[MAX_DESCRIPTION];
    float price;

    printf("Enter menu item ID: ");
    scanf("%d", &id);
    getchar(); // Consume newline

    printf("Enter menu item name: ");
    fgets(name, MAX_STRING, stdin);
    name[strcspn(name, "\n")] = 0; // Remove newline

    printf("Enter menu item description: ");
    fgets(description, MAX_DESCRIPTION, stdin);
    description[strcspn(description, "\n")] = 0; // Remove newline

    printf("Enter menu item price: ");
    scanf("%f", &price);
    getchar(); // Consume newline

    addMenuItem(id, name, description, price);
    printf("Menu item added successfully!\n");
}

void processDeleteMenuItem() {
    int id;

    printf("Enter the ID of the menu item to delete: ");
    scanf("%d", &id);
    getchar(); // Consume newline

    removeMenuItem(id);
}

void processResetMenu() {
    char confirmation;

    printf("Are you sure you want to reset the menu? This will delete all menu items. (y/n): ");
    scanf(" %c", &confirmation);
    getchar(); // Consume newline

    if (confirmation == 'y' || confirmation == 'Y') {
        resetMenu();
    } else {
        printf("Menu reset cancelled.\n");
    }
}

void processAddOrder() {
    char customerName[MAX_STRING];
    int itemId;
    OrderItem* head = NULL;
    OrderItem* tail = NULL;
    float total = 0.0;

    printf("Enter customer name: ");
    fgets(customerName, MAX_STRING, stdin);
    customerName[strcspn(customerName, "\n")] = 0; // Remove newline

    printf("Enter item IDs (0 to finish): ");

    while (1) {
        scanf("%d", &itemId);
        if (itemId == 0) break;

        MenuItem* menuItem = findMenuItem(itemId);
        if (menuItem == NULL) {
            printf("Item with ID %d not found. Please try again.\n", itemId);
            continue;
        }

        // Create new order item
        OrderItem* newItem = (OrderItem*)malloc(sizeof(OrderItem));
        if (newItem == NULL) {
            fprintf(stderr, "Memory allocation failed\n");
            exit(EXIT_FAILURE);
        }

        newItem->item = menuItem;
        newItem->next = NULL;
        total += menuItem->price;

        // Add to linked list
        if (head == NULL) {
            head = newItem;
            tail = newItem;
        } else {
            tail->next = newItem;
            tail = newItem;
        }
    }
    getchar(); // Consume newline

    if (head == NULL) {
        printf("No items selected. Order cancelled.\n");
        return;
    }

    // Create new order
    Order* newOrder = (Order*)malloc(sizeof(Order));
    if (newOrder == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    newOrder->id = orderIdCounter++;
    strncpy(newOrder->customerName, customerName, MAX_STRING - 1);
    newOrder->customerName[MAX_STRING - 1] = '\0';
    newOrder->items = head;
    newOrder->completed = false;
    newOrder->total = total;

    // Add to queue
    enqueue(newOrder);
    printf("Order added successfully!\n");
}

void processNextOrder() {
    if (isQueueEmpty()) {
        printf("No orders to process.\n");
        return;
    }

    Order* order = dequeue();
    printf("Processing order for %s...\n", order->customerName);

    order->completed = true;
    push(order);

    printf("Order processed successfully!\n");
}

void displayOrders() {
    displayQueue();
    displayStack();
}

void processDeleteOrder() {
    if (isQueueEmpty()) {
        printf("No active orders to delete.\n");
        return;
    }

    int id;
    printf("Enter the ID of the order to cancel: ");
    scanf("%d", &id);
    getchar(); // Consume newline

    removeOrderFromQueue(id);
}

void calculateTotalRevenue() {
    if (isStackEmpty()) {
        printf("No completed orders to calculate revenue.\n");
        return;
    }

    printf("\n--- Total Revenue ---\n");

    // Create a temp stack to restore completed orders after calculation
    Stack tempStack;
    tempStack.top = NULL;
    tempStack.size = 0;

    float totalRevenue = 0.0;

    // Pop all items from completedOrders, calculate revenue, and push to tempStack
    while (!isStackEmpty()) {
        Order* order = pop();

        printf("Order %d: $%.2f\n", order->id, order->total);
        totalRevenue += order->total;

        // Push to temporary stack
        Node* newNode = (Node*)malloc(sizeof(Node));
        if (newNode == NULL) {
            fprintf(stderr, "Memory allocation failed\n");
            exit(EXIT_FAILURE);
        }

        newNode->order = order;
        newNode->next = tempStack.top;
        tempStack.top = newNode;
        tempStack.size++;
    }

    printf("Total Sold: $%.2f\n", totalRevenue);

    // Restore completed orders from tempStack
    while (tempStack.top != NULL) {
        Node* temp = tempStack.top;
        Order* order = temp->order;

        push(order);

        tempStack.top = tempStack.top->next;
        free(temp);
    }
}

void displayMainMenu() {
    printf("\n--- Restaurant Order Management System ---\n");
    printf("1. Display Menu\n");
    printf("2. Add Menu Item\n");
    printf("3. Delete Menu Item\n");
    printf("4. Reset Menu\n");
    printf("5. Add New Order\n");
    printf("6. Process Next Order\n");
    printf("7. Display Orders\n");
    printf("8. Delete Order\n");
    printf("9. Calculate Total Amount of Sold Orders\n");
    printf("10. Save Completed Orders to File\n");
    printf("11. Save Menu to File\n");
    printf("12. Exit\n");
    printf("Enter your choice: ");
}

int main() {
    // Initialize data structures
    initDynamicArrayList();
    initQueue();
    initStack();

    // Load menu from file
    loadMenuFromFile(MENU_FILE);

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
                processAddMenuItem();
                break;
            case 3:
                processDeleteMenuItem();
                break;
            case 4:
                processResetMenu();
                break;
            case 5:
                processAddOrder();
                break;
            case 6:
                processNextOrder();
                break;
            case 7:
                displayOrders();
                break;
            case 8:
                processDeleteOrder();
                break;
            case 9:
                calculateTotalRevenue();
                break;
            case 10:
                saveCompletedOrdersToFile(ORDERS_FILE);
                break;
            case 11:
                saveMenuToFile(MENU_FILE);
                break;
            case 12:
                running = false;
                printf("Exiting the program... Goodbye!\n");
                break;
            default:
                printf("Invalid choice. Please try again.\n");
        }
    }

    // Clean up
    destroyDynamicArrayList();
    destroyQueue();
    destroyStack();

    return 0;
}
