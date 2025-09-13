#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define ITEMS_FILE "items.txt"   // items.txt sits next to your .c/.exe

// Look up a product by code in items.txt (format: code, name, price)
int findItemByCode(int code, char *outName, float *outPrice) {
    FILE *fp = fopen(ITEMS_FILE, "r");
    if (!fp) {
        printf(" Could not open %s\n", ITEMS_FILE);
        return 0;
    }

    int fileCode;
    char name[50];
    float price;
    int found = 0;

    // Each line: 101, Pen, 10
    while (fscanf(fp, " %d , %49[^,] , %f", &fileCode, name, &price) == 3) {
        // trim trailing spaces from name (optional)
        size_t n = strlen(name);
        while (n && (name[n-1] == ' ' || name[n-1] == '\t')) name[--n] = '\0';

        if (fileCode == code) {
            if (outName)  strcpy(outName, name);
            if (outPrice) *outPrice = price;
            found = 1;
            break;
        }
        // eat remainder of line
        int c;
        while ((c = fgetc(fp)) != '\n' && c != EOF) {}
    }

    fclose(fp);
   return found;
}
// ================= STRUCTURES =================
typedef struct item {
    char name[50];
    int quantity;
    float price;
    float total;
    int product_code;
    struct item *next;
} item;

typedef struct {
    int custID;
    char name[50];
    char phone[15];
    char email[50];
    char address[100];
} Customer;

typedef struct {
    int receiptID;
    int custID;
    int itemID;
    int quantity;
    char date[20];     // YYYY-MM-DD
    float amount;
} Receipt;

typedef struct SalesNode {
    int key;                 // custID or itemID
    float totalAmount;
    int totalQuantity;
    struct SalesNode *next;
} SalesNode;

// ================= GLOBALS =================
item *head = NULL;
int globalReceiptID = 1;

// ================= HELPERS =================
void getDate(char *buffer) {
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    sprintf(buffer, "%04d-%02d-%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
}

void toLower(char *str) {
    for (int i = 0; str[i]; i++) {
        if (str[i] >= 'A' && str[i] <= 'Z')
            str[i] += 32;
    }
}
item *createNode() {
    item *newnode = (item *)malloc(sizeof(item));
    if (!newnode) { printf("Memory allocation failed!\n"); return NULL; }

    // Ask only for product code and quantity
    printf("\nEnter product code: ");
    scanf("%d", &newnode->product_code);

    // Auto-fill name & price from items.txt
    if (!findItemByCode(newnode->product_code, newnode->name, &newnode->price)) {
        printf("❌ Product code %d not found in %s\n", newnode->product_code, ITEMS_FILE);
        free(newnode);
        return NULL;
    }

    printf("Product: %s | Price: %.2f\n", newnode->name, newnode->price);

    printf("Enter product quantity: ");
    scanf("%d", &newnode->quantity);

    newnode->total = newnode->price * newnode->quantity;
    newnode->next = NULL;
    return newnode;
}

void addItem() {
    item *newnode = createNode();
    if (!newnode) return;

    if (head == NULL) head = newnode;
    else {
        item *temp = head;
        while (temp->next != NULL) temp = temp->next;
        temp->next = newnode;
    }
    printf("✅ Item added successfully!\n");
}

void displayBill() {
    if (head == NULL) { printf("\n❌ No items in the bill.\n"); return; }

    float grand_total = 0;
    printf("\n------------------ BILL ------------------\n");
    printf("%-20s %-10s %-10s %-10s\n", "Product Name", "Qty", "Price", "Total");
    printf("------------------------------------------\n");

    item *temp = head;
    while (temp != NULL) {
        printf("%-20s %-10d %-10.2f %-10.2f\n", temp->name, temp->quantity, temp->price, temp->total);
        grand_total += temp->total;
        temp = temp->next;
    }

    printf("------------------------------------------\n");
    printf("Grand Total: %.2f\n", grand_total);
    printf("------------------------------------------\n");
}


void finalizeBill(int custID) {
    if (head == NULL) {
        printf("❌ No items to save.\n");
        return;
    }

    FILE *fp_txt = fopen("bill.txt", "a");
    FILE *fp_receipts = fopen("receipts.txt", "a");  // Open as text file in append mode
    if (!fp_txt || !fp_receipts) {
        printf("Error opening file!\n");
        return;
    }

    fprintf(fp_txt, "------------------ BILL ------------------\n");
    fprintf(fp_txt, "%-20s %-10s %-10s %-10s\n", "Product Name", "Qty", "Price", "Total");
    fprintf(fp_txt, "------------------------------------------\n");

    item *temp = head;
    float grand_total = 0;
    char today[20]; getDate(today);

    while (temp != NULL) {
        fprintf(fp_txt, "%-20s %-10d %-10.2f %-10.2f\n",
                temp->name, temp->quantity, temp->price, temp->total);
        grand_total += temp->total;

        // Write to receipts.txt in readable format
        fprintf(fp_receipts, "%d %d %d %d %s %.2f\n",
                globalReceiptID, custID, temp->product_code, temp->quantity, today, temp->total);

        temp = temp->next;
    }

    fprintf(fp_txt, "------------------------------------------\n");
    fprintf(fp_txt, "Grand Total: %.2f\n", grand_total);
    fprintf(fp_txt, "Receipt ID: %d\n", globalReceiptID);
    fprintf(fp_txt, "Date: %s\n", today);
    fprintf(fp_txt, "------------------------------------------\n");

    fclose(fp_txt);
    fclose(fp_receipts);

    printf("✅ Bill finalized and saved! (ReceiptID: %d)\n", globalReceiptID);
    globalReceiptID++;
}

void freeItems() {
    item *temp;
    while (head != NULL) {
        temp = head;
        head = head->next;
        free(temp);
    }
}

void billingMenu() {
    int choice, custID;
    while (1) {
        printf("\n==== Billing Menu ====\n");
        printf("1. Add Item\n");
        printf("2. Display Bill\n");
        printf("3. Finalize Bill (Save)\n");
        printf("4. Back\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        if (choice == 1) addItem();
        else if (choice == 2) displayBill();
        else if (choice == 3) {
            printf("Enter Customer ID for this bill: ");
            scanf("%d", &custID);
            finalizeBill(custID);
            freeItems();
        }
        else if (choice == 4) break;
        else printf("❌ Invalid choice!\n");
    }
}
// ================= MODULE 2: CUSTOMER =================
void addCustomer() {
    FILE *fp = fopen("customers.txt", "a");
    if (!fp) {
        printf("Error opening file!\n");
        return;
    }
    Customer c;
    printf("Enter Customer ID: "); scanf("%d", &c.custID); getchar();
    printf("Enter Name: "); fgets(c.name, 50, stdin); c.name[strcspn(c.name, "\n")] = 0;
    printf("Enter Phone: "); scanf("%s", c.phone);
    printf("Enter Email: "); scanf("%s", c.email); getchar();
    printf("Enter Address: "); fgets(c.address, 100, stdin); c.address[strcspn(c.address, "\n")] = 0;
    // Replace spaces in address with underscores (optional but helps reading)
    for (int i = 0; c.address[i]; i++)
        if (c.address[i] == ' ') c.address[i] = '_';
    fprintf(fp, "%d %s %s %s %s\n",
            c.custID, c.name, c.phone, c.email, c.address);
    fclose(fp);
    printf("✅ Customer added successfully!\n");
}
void searchCustomer() {
    FILE *fp = fopen("customers.txt", "r");
    if (!fp) {
        printf("Error opening file!\n");
        return;
    }
    char key[50], keyLower[50];
    printf("Enter Name or Phone: ");
    scanf(" %[^\n]", key);
    strcpy(keyLower, key); toLower(keyLower);
    int custID;
    char name[50], phone[15], email[50], address[100];
    int found = 0;

    while (fscanf(fp, "%d %49s %14s %49s %[^\n]",
                  &custID, name, phone, email, address) == 5) {
        char nameLower[50];
        strcpy(nameLower, name);
        toLower(nameLower);
        if (strstr(nameLower, keyLower) || strcmp(phone, key) == 0) {
            printf("\n✅ Customer Found:\n");
            printf("ID:%d Name:%s Phone:%s Email:%s Address:%s\n",
                   custID, name, phone, email, address);
            found = 1;
            break;
        }
    }
    if (!found)
        printf("❌ Customer not found!\n");

    fclose(fp);
}
void updateCustomer() {
    FILE *fp = fopen("customers.txt", "r");
    if (!fp) {
        printf("❌ Error opening customers.txt for reading!\n");
        return;
    }
    Customer customers[100];  // Store all customers in memory
    int count = 0;
    // Read all customers from file
    while (fscanf(fp, "%d %49s %14s %49s %99s",
                  &customers[count].custID,
                  customers[count].name,
                  customers[count].phone,
                  customers[count].email,
                  customers[count].address) == 5) {
        count++;
    }
    fclose(fp);
    int id, found = 0;
    printf("Enter Customer ID to update: ");
    scanf("%d", &id);
    // Find and update customer
    for (int i = 0; i < count; i++) {
        if (customers[i].custID == id) {
            printf("Updating customer: %s\n", customers[i].name);

            printf("New Phone: ");
            scanf("%s", customers[i].phone);

            printf("New Email: ");
            scanf("%s", customers[i].email); getchar();

            printf("New Address: ");
            fgets(customers[i].address, 100, stdin);
            customers[i].address[strcspn(customers[i].address, "\n")] = 0;

            found = 1;
            break;
        }
    }

    if (!found) {
        printf("❌ Customer ID not found!\n");
        return;
    }

    // Rewrite the file with updated data
    fp = fopen("customers.txt", "w");
    if (!fp) {
        printf("❌ Error opening customers.txt for writing!\n");
        return;
    }

    for (int i = 0; i < count; i++) {
        fprintf(fp, "%d %s %s %s %s\n",
                customers[i].custID,
                customers[i].name,
                customers[i].phone,
                customers[i].email,
                customers[i].address);
    }

    fclose(fp);
    printf("✅ Customer updated successfully!\n");
}
void fetchReceiptHistory(int custID) {
    FILE *fp = fopen("receipts.txt", "rb");
    if (!fp) { printf("Error opening receipts file!\n"); return; }
    int receiptID, fileCustID, itemID, quantity;
    char date[20];
    float amount;
    int found=0;
while (fscanf(fp, "%d %d %d %d %s %f",
              &receiptID, &fileCustID, &itemID, &quantity, date, &amount) == 6) {
    if (fileCustID == custID) {
        printf("Receipt:%d Item_code:%d Qty:%d Amount:%.2f Date:%s\n",
               receiptID, itemID, quantity, amount, date);
    found=1;
    }
}
    if (found==0) printf("❌ No receipts found for this customer.\n");
    fclose(fp);
}
void customerMenu() {
    int ch;
    while (1) {
        printf("\n===== Customer Management =====\n");
        printf("1. Add Customer\n");
        printf("2. Search Customer\n");
        printf("3. Update Customer\n");
        printf("4. Fetch Receipt History\n");
        printf("5. Back\n");
        printf("Enter choice: "); scanf("%d", &ch);

        if (ch == 1) addCustomer();
        else if (ch == 2) searchCustomer();
        else if (ch == 3) updateCustomer();
        else if (ch == 4) {
            int id; printf("Enter Customer ID: "); scanf("%d", &id);
            fetchReceiptHistory(id);
        }
        else if (ch == 5) break;
        else printf("Invalid choice!\n");
    }
}
// ================= MODULE 3: REPORTS =================
void salesReportByPeriod(const char *prefix, const char *label) {
    FILE *fp = fopen("receipts.txt", "r");
    FILE *out = fopen("report.txt", "a");
    if (!fp || !out) { printf("Error opening files!\n"); return; }

    int receiptID, custID, itemID, quantity;
    char date[20];
    float amount;
    float total = 0;
    int count = 0;

    fprintf(out, "\n=== %s Sales Report for %s ===\n", label, prefix);
    printf("\n=== %s Sales Report for %s ===\n", label, prefix);

    while (fscanf(fp, "%d %d %d %d %19s %f", &receiptID, &custID, &itemID, &quantity, date, &amount) == 6) {
        if (strncmp(date, prefix, strlen(prefix)) == 0) {
            fprintf(out, "Receipt:%d Cust:%d Amount:%.2f Date:%s\n", receiptID, custID, amount, date);
            printf("Receipt:%d Cust:%d Amount:%.2f Date:%s\n", receiptID, custID, amount, date);
            total += amount;
            count++;
        }
    }
    fprintf(out, "Transactions:%d Total:%.2f\n", count, total);
    printf("Transactions:%d Total:%.2f\n", count, total);

    fclose(fp);
    fclose(out);
}
void productWiseReport() {
    FILE *fp = fopen("receipts.txt", "r");
    FILE *out = fopen("report.txt", "a");
    if (!fp || !out) return;

    typedef struct SalesNode {
        int key; // itemID
        float totalAmount;
        int totalQuantity;
        struct SalesNode *next;
    } SalesNode;

    SalesNode *head = NULL;

    int receiptID, custID, itemID, quantity;
    char date[20];
    float amount;

    while (fscanf(fp, "%d %d %d %d %19s %f", &receiptID, &custID, &itemID, &quantity, date, &amount) == 6) {
        SalesNode *node = head;
        while (node && node->key != itemID) node = node->next;
        if (!node) {
            node = (SalesNode*)malloc(sizeof(SalesNode));
            node->key = itemID;
            node->totalAmount = 0;
            node->totalQuantity = 0;
            node->next = head;
            head = node;
        }
        node->totalAmount += amount;
        node->totalQuantity += quantity;
    }
    fprintf(out, "\n=== Product-wise Report ===\n");
    printf("\n=== Product-wise Report ===\n");
    for (SalesNode *p = head; p; p = p->next) {
        fprintf(out, "Item:%d Qty:%d Sales:%.2f\n", p->key, p->totalQuantity, p->totalAmount);
        printf("Item:%d Qty:%d Sales:%.2f\n", p->key, p->totalQuantity, p->totalAmount);
    }
    fclose(fp); 
    fclose(out);
}
void customerSpendingReport() {
    FILE *fp = fopen("receipts.txt", "r");
    FILE *out = fopen("report.txt", "a");
    if (!fp || !out) return;
    typedef struct SalesNode {
        int key; // custID
        float totalAmount;
        int totalQuantity;
        struct SalesNode *next;
    } SalesNode;
    SalesNode *head = NULL;
    int receiptID, custID, itemID, quantity;
    char date[20];
    float amount;
    while (fscanf(fp, "%d %d %d %d %19s %f", &receiptID, &custID, &itemID, &quantity, date, &amount) == 6) {
        SalesNode *node = head;
        while (node && node->key != custID) node = node->next;
        if (!node) {
            node = (SalesNode*)malloc(sizeof(SalesNode));
            node->key = custID;
            node->totalAmount = 0;
            node->totalQuantity = 0;
            node->next = head;
            head = node;
        }
        node->totalAmount += amount;
        node->totalQuantity += quantity;
    }
    fprintf(out, "\n=== Customer-wise Report ===\n");
    printf("\n=== Customer-wise Report ===\n");
    for (SalesNode *p = head; p; p = p->next) {
        fprintf(out, "Customer:%d Purchases:%d Spent:%.2f\n", p->key, p->totalQuantity, p->totalAmount);
        printf("Customer:%d Purchases:%d Spent:%.2f\n", p->key, p->totalQuantity, p->totalAmount);
    }
    fclose(fp); 
    fclose(out);
}
void totalIncomeByRange(const char *startDate, const char *endDate) {
    FILE *fp = fopen("receipts.txt", "r");
    if (!fp) return;
    int receiptID, custID, itemID, quantity;
    char date[20];
    float amount;
    float total = 0;
    printf("\n=== Income from %s to %s ===\n", startDate, endDate);
    while (fscanf(fp, "%d %d %d %d %19s %f", &receiptID, &custID, &itemID, &quantity, date, &amount) == 6) {
        if (strcmp(date, startDate) >= 0 && strcmp(date, endDate) <= 0) {
            total += amount;
        }
    }
    printf("Total Income: %.2f\n", total);
    fclose(fp);
}
void reportMenu() {
    int choice;
    while (1) {
        printf("\n===== Reports Menu =====\n");
        printf("1. Daily Sales Report\n");
        printf("2. Monthly Sales Report\n");
        printf("3. Product-wise Sales Report\n");
        printf("4. Customer-wise Spending Report\n");
        printf("5. Total Income (date range)\n");
        printf("6. Back\n");
        printf("Enter choice: "); scanf("%d", &choice);
        if (choice == 1) {
            char date[20]; printf("Enter date (YYYY-MM-DD): "); scanf("%s", date);
            salesReportByPeriod(date, "Daily");
        } else if (choice == 2) {
            char month[10]; printf("Enter month (YYYY-MM): "); scanf("%s", month);
            salesReportByPeriod(month, "Monthly");
        } else if (choice == 3) productWiseReport();
        else if (choice == 4) customerSpendingReport();
        else if (choice == 5) {
            char start[20], end[20];
            printf("Start date (YYYY-MM-DD): "); scanf("%s", start);
            printf("End date (YYYY-MM-DD): "); scanf("%s", end);
            totalIncomeByRange(start, end);
        } else if (choice == 6) break;
        else printf("Invalid choice!\n");
    }
}
// ================= MAIN =================
int main() {
    int choice;
    while (1) {
        printf("\n===== Retail Store System =====\n");
        printf("1. Billing\n");
        printf("2. Customer Management\n");
        printf("3. Reports\n");
        printf("4. Exit\n");
        printf("Enter choice: "); scanf("%d", &choice);
        if (choice == 1) billingMenu();
        else if (choice == 2) customerMenu();
        else if (choice == 3) reportMenu();
        else if (choice == 4) break;
        else printf("Invalid choice!\n");
    }
    return 0;
}