/* newtest_bill_sidebar.c
   Retail Store System with Billing, Customer, Reports
   and permanent right-side BILL sidebar (without sys/ioctl.h).
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define ITEMS_FILE "items.txt"
#define RECEIPTS_FILE "receipts.txt"

/* ========== Utility Functions ========== */
void clear_screen() {
    printf("\033[2J\033[H");
}

void move_cursor(int r, int c) {
    printf("\033[%d;%dH", r, c);
}

void print_left_header(const char *title) {
    move_cursor(1,1);
    printf("=== Retail Store System === %s\n", title? title: "");
    printf("-----------------------------------------------\n");
}

/* ========== Item Linked List ========== */
typedef struct item {
    char name[50];
    int quantity;
    float price;
    float total;
    int product_code;
    struct item *next;
} item;

item *head=NULL;
int globalReceiptID=1;

/* ========== Date Helper ========== */
void getDate(char *buffer) {
    time_t t=time(NULL);
    struct tm tm=*localtime(&t);
    sprintf(buffer,"%04d-%02d-%02d", tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday);
}

/* ========== Item Functions ========== */
int findItemByCode(int code, char *outName, float *outPrice) {
    FILE *fp=fopen(ITEMS_FILE,"r");
    if(!fp) return 0;
    int fileCode; char name[50]; float price;
    while(fscanf(fp," %d , %49[^,] , %f",&fileCode,name,&price)==3) {
        if(fileCode==code) {
            if(outName) strcpy(outName,name);
            if(outPrice) *outPrice=price;
            fclose(fp);
            return 1;
        }
        int c; while((c=fgetc(fp))!='\n' && c!=EOF){}
    }
    fclose(fp); return 0;
}

item* createNode() {
    item* newnode=(item*)malloc(sizeof(item));
    if(!newnode) return NULL;
    printf("Enter product code: "); scanf("%d",&newnode->product_code);
    if(!findItemByCode(newnode->product_code,newnode->name,&newnode->price)){
        printf("❌ Product not found\n"); free(newnode); return NULL;
    }
    printf("Product: %s Price: %.2f\n",newnode->name,newnode->price);
    printf("Enter qty: "); scanf("%d",&newnode->quantity);
    newnode->total=newnode->price*newnode->quantity;
    newnode->next=NULL; return newnode;
}

void addItem() {
    item* n=createNode(); if(!n) return;
    if(!head) head=n;
    else {item* t=head; while(t->next) t=t->next; t->next=n;}
    printf("✅ Item added\n");
}

/* ========== Sidebar BILL Display ========== */
void print_bill_sidebar(int start_row) {
    int col = 60;
    int boxWidth = 40;
    move_cursor(start_row, col);
    printf("+%.*s+", boxWidth-2, "--------------------------------------");
    move_cursor(start_row+1, col);
    printf("| Current BILL (Cart)                 |");
    move_cursor(start_row+2, col);
    printf("+%.*s+", boxWidth-2, "--------------------------------------");

    item* t=head; int row=start_row+3;
    float g=0; int shown=0;
    while(t && shown<10) {
        char line[100];
        snprintf(line,sizeof(line),"%s x%d = %.2f",t->name,t->quantity,t->total);
        if((int)strlen(line)>boxWidth-4) line[boxWidth-4]=0;
        move_cursor(row++,col);
        printf("| %-*s |",boxWidth-4,line);
        g+=t->total; t=t->next; shown++;
    }
    for(; shown<10; shown++,row++){
        move_cursor(row,col);
        printf("| %-*s |",boxWidth-4,"");
    }
    move_cursor(row,col);
    printf("+%.*s+", boxWidth-2, "--------------------------------------");
    move_cursor(row+1,col);
    printf("| Grand Total: %-20.2f |",g);
    move_cursor(row+2,col);
    printf("+%.*s+", boxWidth-2, "--------------------------------------");
}

/* ========== Billing Functions ========== */
void displayBill() {
    if(!head){printf("❌ Empty bill\n");return;}
    float g=0; printf("\n%-20s %-5s %-7s %-7s\n","Product","Qty","Price","Total");
    item*t=head; while(t){printf("%-20s %-5d %-7.2f %-7.2f\n",
        t->name,t->quantity,t->price,t->total); g+=t->total; t=t->next;}
    printf("Grand Total: %.2f\n",g);
}

void finalizeBill(int custID) {
    if(!head){printf("❌ No items\n");return;}
    FILE *fp=fopen(RECEIPTS_FILE,"a");
    item*t=head; char date[20]; getDate(date);
    while(t){
        fprintf(fp,"%d %d %d %d %s %.2f\n",globalReceiptID,custID,
                t->product_code,t->quantity,date,t->total);
        t=t->next;
    }
    fclose(fp);
    printf("✅ Bill saved (ReceiptID:%d)\n",globalReceiptID);
    globalReceiptID++;
    while(head){t=head; head=head->next; free(t);}
}

/* ========== Customer Management ========== */
typedef struct {
    int custID;
    char name[50];
    char phone[15];
    char email[50];
    char address[100];
} Customer;

void addCustomer() {
    FILE *fp=fopen("customers.txt","a");
    if(!fp){printf("Error\n");return;}
    Customer c;
    printf("Enter ID: "); scanf("%d",&c.custID); getchar();
    printf("Enter Name: "); fgets(c.name,50,stdin); c.name[strcspn(c.name,"\n")]=0;
    printf("Enter Phone: "); scanf("%s",c.phone);
    printf("Enter Email: "); scanf("%s",c.email); getchar();
    printf("Enter Address: "); fgets(c.address,100,stdin); c.address[strcspn(c.address,"\n")]=0;
    for(int i=0;c.address[i];i++) if(c.address[i]==' ') c.address[i]='_';
    fprintf(fp,"%d %s %s %s %s\n",c.custID,c.name,c.phone,c.email,c.address);
    fclose(fp);
    printf("✅ Customer added\n");
}

void searchCustomer() {
    FILE *fp=fopen("customers.txt","r");
    if(!fp){printf("Error\n");return;}
    char key[50]; printf("Enter Name/Phone: "); scanf(" %[^\n]",key);
    int id; char name[50],phone[15],email[50],addr[100]; int found=0;
    while(fscanf(fp,"%d %49s %14s %49s %[^\n]",&id,name,phone,email,addr)==5){
        if(strstr(name,key)||strcmp(phone,key)==0){
            printf("Found ID:%d Name:%s Phone:%s Email:%s Addr:%s\n",id,name,phone,email,addr);
            found=1; break;
        }
    }
    if(!found) printf("❌ Not found\n");
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
        printf("Error opening customers.txt for writing!\n");
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

void fetchReceiptHistory(int cid) {
    FILE *fp=fopen(RECEIPTS_FILE,"r");
    if(!fp){printf("No receipts\n");return;}
    int rid, cust, item, qty; char date[20]; float amt; int found=0;
    while(fscanf(fp,"%d %d %d %d %19s %f",&rid,&cust,&item,&qty,date,&amt)==6){
        if(cust==cid){
            printf("Receipt:%d Item:%d Qty:%d Amt:%.2f Date:%s\n",rid,item,qty,amt,date);
            found=1;
        }
    }
    if(!found) printf("❌ No receipts for this customer\n");
    fclose(fp);
}

/* ========== Reports ========== */
// void reportMenu() {
//     int ch;
//     while(1){
//         clear_screen(); print_left_header(" - Reports"); print_bill_sidebar(2);
//         move_cursor(6,1);
//         printf("1.Show receipts\n2.Back\nChoice: "); scanf("%d",&ch);
//         if(ch==1){
//             FILE *fp=fopen(RECEIPTS_FILE,"r");
//             if(!fp) printf("No receipts\n");
//             else {char l[256]; while(fgets(l,sizeof(l),fp)) printf("%s",l); fclose(fp);}
//             getchar(); getchar();
//         } else if(ch==2) break;
//     }
// }
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

/* ========== Screens ========== */
void render_screen(const char *title) {
    clear_screen();
    print_left_header(title);
    print_bill_sidebar(2);
    move_cursor(6,1);
}

/* Billing menu */
void billingMenu() {
    int ch,id;
    while(1){
        render_screen(" - Billing");
        printf("1.Add Item\n2.Display Bill\n3.Finalize Bill\n4.Back\nChoice: ");
        scanf("%d",&ch);
        if(ch==1){addItem(); getchar(); getchar();}
        else if(ch==2){displayBill(); getchar(); getchar();}
        else if(ch==3){printf("CustomerID: "); scanf("%d",&id);
            finalizeBill(id); getchar(); getchar();}
        else if(ch==4) break;
    }
}

/* Customer menu */
void customerMenu() {
    int ch;
    while(1){
        render_screen(" - Customer");
        printf("1.Add Customer\n2.Search Customer\n3.Update customer\n4.Fetch Receipts\n5.Back\nChoice: ");
        scanf("%d",&ch);
        if(ch==1){addCustomer(); getchar(); getchar();}
        else if(ch==2){searchCustomer(); getchar(); getchar();}
        else if(ch==3)updateCustomer();
        else if(ch==4){int id; printf("Enter ID: "); scanf("%d",&id); fetchReceiptHistory(id); getchar(); getchar();}
        else if(ch==5) break;
    }
}



/* main */
int main() {
    int ch;
    while(1){
        render_screen("");
        printf("1.Billing\n2.Customer Management\n3.Reports\n4.Exit\nChoice: ");
        scanf("%d",&ch);
        if(ch==1) billingMenu();
        else if(ch==2) customerMenu();
        else if(ch==3) reportMenu();
        else if(ch==4) break;
    }
    clear_screen(); return 0;
}
