/* newtest_bill_sidebar_discount.c
   Retail Store System with Billing, Customer, Reports
   - Permanent right-side BILL sidebar (no sys/ioctl.h)
   - 5% discount for bills >= 1000
   - Colored, user-friendly output
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#ifdef _WIN32
#include <windows.h>
#endif

#define ITEMS_FILE     "items.txt"
#define RECEIPTS_FILE  "receipts.txt"
#define CUSTOMERS_FILE "customers.txt"

/* ====================== ANSI COLORS ======================= */
#define C_RESET  "\033[0m"
#define C_BOLD   "\033[1m"
#define C_DIM    "\033[2m"

#define FG_BLACK   "\033[30m"
#define FG_RED     "\033[31m"
#define FG_GREEN   "\033[32m"
#define FG_YELLOW  "\033[33m"
#define FG_BLUE    "\033[34m"
#define FG_MAGENTA "\033[35m"
#define FG_CYAN    "\033[36m"
#define FG_WHITE   "\033[37m"

#define BG_BLACK   "\033[40m"
#define BG_RED     "\033[41m"
#define BG_GREEN   "\033[42m"
#define BG_YELLOW  "\033[43m"
#define BG_BLUE    "\033[44m"
#define BG_MAGENTA "\033[45m"
#define BG_CYAN    "\033[46m"
#define BG_WHITE   "\033[47m"

/* Pretty helpers */
#define TAG_OK   (FG_GREEN C_BOLD "[OK]" C_RESET)
#define TAG_ERR  (FG_RED   C_BOLD "[ERR]" C_RESET)
#define TAG_INFO (FG_CYAN  C_BOLD "[INFO]" C_RESET)

/* Enable ANSI on legacy Windows (best effort) */
static void enable_ansi_if_windows(void){
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD mode = 0;
        if (GetConsoleMode(hOut, &mode)) {
            mode |= 0x0004; /* ENABLE_VIRTUAL_TERMINAL_PROCESSING */
            SetConsoleMode(hOut, mode);
        }
    }
#endif
}

/* ====================== Utility ======================= */
void clear_screen() { printf("\033[2J\033[H"); }
void move_cursor(int r, int c) { printf("\033[%d;%dH", r, c); }

void color_bar(const char *bg, const char *fg, const char *text, int width) {
    /* Prints a single full-width colored bar with centered text */
    int len = (int)strlen(text);
    if (len > width - 2) len = width - 2;
    int pad = (width - 2 - len) / 2;
    printf("%s%s", bg, fg);
    putchar(' ');
    for (int i=0;i<pad;i++) putchar(' ');
    fwrite(text, 1, len, stdout);
    for (int i=0;i<(width - 2 - pad - len); i++) putchar(' ');
    printf(" " C_RESET);
}

void print_left_header(const char *title) {
    move_cursor(1,1);
    printf(FG_WHITE C_BOLD "=== Retail Store System ===%s%s\n" C_RESET,
           title && title[0] ? " " : "",
           title && title[0] ? title : "");
    printf(FG_BLUE "------------------------------------------------------------" C_RESET "\n");
}

/* ====================== Item Linked List ======================= */
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

/* ====================== Date Helper ======================= */
void getDate(char *buffer) {
    time_t t=time(NULL);
    struct tm tm=*localtime(&t);
    sprintf(buffer,"%04d-%02d-%02d", tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday);
}

/* ====================== Item DB Functions ======================= */
// int findItemByCode(int code, char *outName, float *outPrice) {
//     FILE *fp=fopen(ITEMS_FILE,"r");
//     if(!fp) return 0;
//     int fileCode; char name[50]; float price;
//     while(fscanf(fp," %d , %49[^,] , %f",&fileCode,name,&price)==3) {
//         if(fileCode==code) {
//             if(outName) strcpy(outName,name);
//             if(outPrice) *outPrice=price;
//             fclose(fp);
//             return 1;
//         }
//         int c; while((c=fgetc(fp))!='\n' && c!=EOF){} /* skip rest of line */
//     }
//     fclose(fp); return 0;
// }
int findItemByName(const char *searchName, int *outCode, float *outPrice) {
    FILE *fp=fopen(ITEMS_FILE,"r");
    if(!fp) return 0;
    int fileCode; char name[50]; float price;
    while(fscanf(fp," %d , %49[^,] , %f",&fileCode,name,&price)==3) {
        if(strcasecmp(name, searchName)==0) {  // case-insensitive match
            if(outCode) *outCode = fileCode;
            if(outPrice) *outPrice = price;
            fclose(fp);
            return 1;
        }
        int c; while((c=fgetc(fp))!='\n' && c!=EOF){} /* skip rest of line */
    }
    fclose(fp); return 0;
}


/* ====================== Cart Ops ======================= */
item* createNode() {
    item* newnode=(item*)malloc(sizeof(item));
    if(!newnode) return NULL;
    printf(FG_YELLOW "Enter product name: " C_RESET);
    if (scanf("%s",newnode->name)!=1){ free(newnode); return NULL; }

    if(!findItemByName(newnode->name,&newnode->product_code,&newnode->price)){
        printf("%s Product not found\n", TAG_ERR);
        free(newnode); return NULL;
    }
    printf(FG_CYAN "Product: " C_RESET "%s  " FG_CYAN "Price: " C_RESET "%.2f\n",
           newnode->name,newnode->price);
    printf(FG_YELLOW "Enter qty: " C_RESET);
    if (scanf("%d",&newnode->quantity)!=1){ free(newnode); return NULL; }

    newnode->total=newnode->price*newnode->quantity;
    newnode->next=NULL; return newnode;
}

void addItem() {
    render_screen("");
    item* n=createNode(); if(!n) return;
    if(!head) head=n;
    else {item* t=head; while(t->next) t=t->next; t->next=n;}
    printf("%s Item added to cart\n", TAG_OK);
}

/* ====================== Sidebar BILL Display ======================= */
void print_bill_sidebar(int start_row) {
    int col = 60;
    int boxWidth = 40;

    /* Header */
    move_cursor(start_row, col);
    printf(FG_BLUE "+%.*s+\n" C_RESET, boxWidth-2, "--------------------------------------");
    move_cursor(start_row+1, col);
    printf(FG_BLUE "|" C_RESET);
    color_bar(BG_CYAN, FG_BLACK C_BOLD, "CURRENT BILL", boxWidth-2);
    printf(FG_BLUE "|\n" C_RESET);
    move_cursor(start_row+2, col);
    printf(FG_BLUE "+%.*s+\n" C_RESET, boxWidth-2, "--------------------------------------");

    /* Lines */
    item* t=head; int row=start_row+3;
    float g=0; int shown=0;
    while(t && shown<10) {
        char line[100];
        snprintf(line,sizeof(line),"%s x%d = %.2f",t->name,t->quantity,t->total);
        if((int)strlen(line)>boxWidth-4) line[boxWidth-4]=0;
        move_cursor(row++,col);
        printf(FG_BLUE "|" C_RESET " %-*s " FG_BLUE "|" C_RESET "\n",boxWidth-4,line);
        g+=t->total; t=t->next; shown++;
    }
    for(; shown<10; shown++,row++){
        move_cursor(row,col);
        printf(FG_BLUE "|" C_RESET " %-*s " FG_BLUE "|" C_RESET "\n",boxWidth-4,"");
    }

    /* Footer with discount preview */
    float discount = (g >= 1000.0f) ? g * 0.05f : 0.0f;
    float payable  = g - discount;

    move_cursor(row,col);
    printf(FG_BLUE "+%.*s+\n" C_RESET, boxWidth-2, "--------------------------------------");
    move_cursor(row+1,col);
    printf(FG_BLUE "|" C_RESET " Subtotal: %-18.2f " FG_BLUE "|\n" C_RESET, g);
    move_cursor(row+2,col);
    printf(FG_BLUE "|" C_RESET " Disc(5%%): %-17.2f " FG_BLUE "|\n" C_RESET, discount);
    move_cursor(row+3,col);
    printf(FG_BLUE "|" C_RESET " " C_BOLD "Payable: %-16.2f" C_RESET " " FG_BLUE "|\n" C_RESET, payable);
    move_cursor(row+4,col);
    printf(FG_BLUE "+%.*s+" C_RESET, boxWidth-2, "--------------------------------------");
}

/* ====================== Billing Functions ======================= */
void displayBill() {
    if(!head){printf("%s Cart is empty\n", TAG_ERR);return;}

    float subtotal=0;
    printf("\n" C_BOLD FG_WHITE "%-20s %-5s %-9s %-9s" C_RESET "\n",
           "Product","Qty","Price","Total");
    printf(FG_BLUE "------------------------------------------------------------" C_RESET "\n");

    item*t=head;
    while(t){
        printf("%-20s %-5d %-9.2f %-9.2f\n",
               t->name,t->quantity,t->price,t->total);
        subtotal+=t->total; t=t->next;
    }
    float discount = (subtotal >= 1000.0f) ? subtotal * 0.05f : 0.0f;
    float payable  = subtotal - discount;

    printf(FG_BLUE "------------------------------------------------------------" C_RESET "\n");
    printf(FG_CYAN  "Subtotal : " C_RESET "%.2f\n", subtotal);
    printf(FG_YELLOW "Discount : " C_RESET "%.2f%s\n", discount,
           discount>0 ? " (5%)" : "");
    printf(C_BOLD FG_GREEN "Payable  : %.2f\n" C_RESET, payable);
}

void freeCart() {
    item *t;
    while(head){ t=head; head=head->next; free(t); }
}

void finalizeBill(int custID) {
    if(!head){printf("%s No items to bill\n", TAG_ERR);return;}

    FILE *fp=fopen(RECEIPTS_FILE,"a");
    if(!fp){ printf("%s Could not open %s\n", TAG_ERR, RECEIPTS_FILE); return; }

    char date[20]; getDate(date);
    float subtotal=0.0f;

    /* Save each item as a line (same format) */
    for(item*t=head; t; t=t->next){
        fprintf(fp,"%d %d %d %d %s %.2f\n",
                globalReceiptID,custID,t->product_code,t->quantity,date,t->total);
        subtotal += t->total;
    }

    /* Apply discount if eligible: store as a negative line with itemID=-1 so
       product-wise reports can ignore it, while income & customer spend include it. */
    float discount = (subtotal >= 1000.0f) ? subtotal * 0.05f : 0.0f;
    float payable  = subtotal - discount;

    if (discount > 0.0f) {
        /* itemID = -1 (DISCOUNT), qty=0 */
        fprintf(fp,"%d %d %d %d %s %.2f\n",
                globalReceiptID, custID, -1, 0, date, -discount);
    }

    /* Optional: write a TOTAL marker line (itemID=-2) — not used in reports, but human readable.
       Comment out if you don't want it. */
    fprintf(fp,"%d %d %d %d %s %.2f\n",
            globalReceiptID, custID, -2, 0, date, payable);

    fclose(fp);

    printf("%s Bill saved " FG_WHITE "(ReceiptID: %d)" C_RESET "\n", TAG_OK, globalReceiptID);
    printf(FG_CYAN "Subtotal: " C_RESET "%.2f, " FG_YELLOW "Discount: " C_RESET "%.2f, "
           FG_GREEN C_BOLD "Payable: " C_RESET "%.2f\n", subtotal, discount, payable);

    globalReceiptID++;
    freeCart();
}

/* ====================== Customer Management ======================= */
typedef struct {
    int custID;
    char name[50];
    char phone[15];
    char email[50];
    char address[100];
} Customer;

void addCustomer() {
    FILE *fp=fopen(CUSTOMERS_FILE,"a");
    if(!fp){printf("%s Unable to open %s\n", TAG_ERR, CUSTOMERS_FILE);return;}
    Customer c;
    printf(FG_YELLOW "Enter ID: " C_RESET); scanf("%d",&c.custID); getchar();
    printf(FG_YELLOW "Enter Name: " C_RESET); fgets(c.name,50,stdin); c.name[strcspn(c.name,"\n")]=0;
    printf(FG_YELLOW "Enter Phone: " C_RESET); scanf("%14s",c.phone);
    printf(FG_YELLOW "Enter Email: " C_RESET); scanf("%49s",c.email); getchar();
    printf(FG_YELLOW "Enter Address: " C_RESET); fgets(c.address,100,stdin); c.address[strcspn(c.address,"\n")]=0;
    for(int i=0;c.address[i];i++) if(c.address[i]==' ') c.address[i]='_';
    fprintf(fp,"%d %s %s %s %s\n",c.custID,c.name,c.phone,c.email,c.address);
    fclose(fp);
    printf("%s Customer added\n", TAG_OK);
}

void searchCustomer() {
    FILE *fp=fopen(CUSTOMERS_FILE,"r");
    if(!fp){printf("%s Open %s failed\n", TAG_ERR, CUSTOMERS_FILE);return;}
    char key[50]; printf(FG_YELLOW "Enter Name/Phone: " C_RESET); scanf(" %[^\n]",key);
    int id; char name[50],phone[15],email[50],addr[100]; int found=0;
    while(fscanf(fp,"%d %49s %14s %49s %99s",&id,name,phone,email,addr)==5){
        if(strstr(name,key)||strcmp(phone,key)==0){
            printf("%s ID:%d  Name:%s  Phone:%s  Email:%s  Addr:%s\n",
                   TAG_INFO,id,name,phone,email,addr);
            found=1; break;
        }
    }
    if(!found) printf("%s Not found\n", TAG_ERR);
    fclose(fp);
}

void updateCustomer() {
    FILE *fp = fopen(CUSTOMERS_FILE, "r");
    if (!fp) { printf("%s Open %s for reading failed!\n", TAG_ERR, CUSTOMERS_FILE); return; }

    Customer customers[100];
    int count = 0;

    while (fscanf(fp, "%d %49s %14s %49s %99s",
                  &customers[count].custID,
                  customers[count].name,
                  customers[count].phone,
                  customers[count].email,
                  customers[count].address) == 5) {
        count++;
        if (count >= 100) break;
    }
    fclose(fp);

    int id, found = 0;
    printf(FG_YELLOW "Enter Customer ID to update: " C_RESET);
    scanf("%d", &id);

    for (int i = 0; i < count; i++) {
        if (customers[i].custID == id) {
            printf(FG_CYAN "Updating: " C_RESET "%s\n", customers[i].name);

            printf(FG_YELLOW "New Phone: " C_RESET);
            scanf("%14s", customers[i].phone);

            printf(FG_YELLOW "New Email: " C_RESET);
            scanf("%49s", customers[i].email); getchar();

            printf(FG_YELLOW "New Address: " C_RESET);
            fgets(customers[i].address, 100, stdin);
            customers[i].address[strcspn(customers[i].address, "\n")] = 0;

            found = 1;
            break;
        }
    }

    if (!found) { printf("%s Customer ID not found!\n", TAG_ERR); return; }

    fp = fopen(CUSTOMERS_FILE, "w");
    if (!fp) { printf("%s Open %s for writing failed!\n", TAG_ERR, CUSTOMERS_FILE); return; }

    for (int i = 0; i < count; i++) {
        fprintf(fp, "%d %s %s %s %s\n",
                customers[i].custID,
                customers[i].name,
                customers[i].phone,
                customers[i].email,
                customers[i].address);
    }

    fclose(fp);
    printf("%s Customer updated successfully!\n", TAG_OK);
}

void fetchReceiptHistory(int cid) {
    FILE *fp=fopen(RECEIPTS_FILE,"r");
    if(!fp){printf("%s No receipts yet\n", TAG_INFO);return;}
    int rid, cust, item, qty; char date[20]; float amt; int found=0;
    while(fscanf(fp,"%d %d %d %d %19s %f",&rid,&cust,&item,&qty,date,&amt)==6){
        if(cust==cid){
            if (item == -1) printf(FG_YELLOW "Receipt:%d DISCOUNT   Amt:%.2f Date:%s\n" C_RESET, rid, amt, date);
            else if (item == -2) printf(FG_GREEN  "Receipt:%d TOTAL      Amt:%.2f Date:%s\n" C_RESET, rid, amt, date);
            else printf("Receipt:%d Item:%d Qty:%d Amt:%.2f Date:%s\n",rid,item,qty,amt,date);
            found=1;
        }
    }
    if(!found) printf("%s No receipts for this customer\n", TAG_INFO);
    fclose(fp);
}

/* ====================== Reports ======================= */
/* Daily/Monthly, includes discounts (negative amounts) and totals (itemID -2) */
void salesReportByPeriod(const char *prefix, const char *label) {
    FILE *fp = fopen(RECEIPTS_FILE, "r");
    FILE *out = fopen("report.txt", "a");
    if (!fp || !out) { printf("%s Error opening files!\n", TAG_ERR); if(fp)fclose(fp); if(out)fclose(out); return; }

    int receiptID, custID, itemID, quantity;
    char date[20];
    float amount;
    float total = 0;
    int count = 0;

    fprintf(out, "\n=== %s Sales Report for %s ===\n", label, prefix);
    printf("\n" C_BOLD "=== %s Sales Report for %s ===" C_RESET "\n", label, prefix);

    while (fscanf(fp, "%d %d %d %d %19s %f", &receiptID, &custID, &itemID, &quantity, date, &amount) == 6) {
        if (strncmp(date, prefix, (int)strlen(prefix)) == 0) {
            fprintf(out, "Receipt:%d Cust:%d Amount:%.2f Date:%s\n", receiptID, custID, amount, date);
            printf("Receipt:%d Cust:%d Amount:%.2f Date:%s\n", receiptID, custID, amount, date);
            total += amount;
            count++;
        }
    }
    fprintf(out, "Transactions:%d Total:%.2f\n", count, total);
    printf(FG_CYAN "Transactions: %d  " FG_GREEN "NET Total: %.2f\n" C_RESET, count, total);

    fclose(fp);
    fclose(out);
}

/* Product-wise: ignore itemID <= 0 so discount/total lines don't pollute items */
void productWiseReport() {
    FILE *fp = fopen(RECEIPTS_FILE, "r");
    FILE *out = fopen("report.txt", "a");
    if (!fp || !out) { if(fp)fclose(fp); if(out)fclose(out); return; }

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
        if (itemID <= 0) continue; /* skip discount/total lines */
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
    printf("\n" C_BOLD "=== Product-wise Report ===" C_RESET "\n");
    for (SalesNode *p = head; p; p = p->next) {
        fprintf(out, "Item:%d Qty:%d Sales:%.2f\n", p->key, p->totalQuantity, p->totalAmount);
        printf("Item:%d Qty:%d Sales:%.2f\n", p->key, p->totalQuantity, p->totalAmount);
    }
    fclose(fp);
    fclose(out);
}

/* Customer-wise: includes discounts (so net spend per customer is accurate) */
void customerSpendingReport() {
    FILE *fp = fopen(RECEIPTS_FILE, "r");
    FILE *out = fopen("report.txt", "a");
    if (!fp || !out) { if(fp)fclose(fp); if(out)fclose(out); return; }

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
        node->totalAmount += amount;          /* discount lines reduce amount */
        if (itemID > 0) node->totalQuantity += quantity; /* ignore -1/-2 for qty */
    }
    fprintf(out, "\n=== Customer-wise Report (NET) ===\n");
    printf("\n" C_BOLD "=== Customer-wise Report (NET) ===" C_RESET "\n");
    for (SalesNode *p = head; p; p = p->next) {
        fprintf(out, "Customer:%d Purchases:%d Spent:%.2f\n", p->key, p->totalQuantity, p->totalAmount);
        printf("Customer:%d Purchases:%d Spent:%.2f\n", p->key, p->totalQuantity, p->totalAmount);
    }
    fclose(fp);
    fclose(out);
}

void totalIncomeByRange(const char *startDate, const char *endDate) {
    FILE *fp = fopen(RECEIPTS_FILE, "r");
    if (!fp) { printf("%s %s not found\n", TAG_ERR, RECEIPTS_FILE); return; }
    int receiptID, custID, itemID, quantity;
    char date[20];
    float amount;
    float total = 0;
    printf("\n" C_BOLD "=== Income from %s to %s ===" C_RESET "\n", startDate, endDate);
    while (fscanf(fp, "%d %d %d %d %19s %f", &receiptID, &custID, &itemID, &quantity, date, &amount) == 6) {
        if (strcmp(date, startDate) >= 0 && strcmp(date, endDate) <= 0) {
            total += amount; /* includes discount and total lines; net effect OK */
        }
    }
    printf(FG_GREEN "NET Income: %.2f\n" C_RESET, total);
    fclose(fp);
}

/* Menu wrapper for reports */
void reportMenu() {
    int choice;
    while (1) {
        printf("\n" C_BOLD "===== Reports Menu =====" C_RESET "\n");
        printf("1. Daily Sales Report\n");
        printf("2. Monthly Sales Report\n");
        printf("3. Product-wise Sales Report\n");
        printf("4. Customer-wise Spending Report (NET)\n");
        printf("5. Total Income (date range)\n");
        printf("6. Back\n");
        printf(FG_YELLOW "Enter choice: " C_RESET); 
        if (scanf("%d", &choice)!=1) { while(getchar()!='\n'); continue; }

        if (choice == 1) {
            char date[20]; printf(FG_YELLOW "Enter date (YYYY-MM-DD): " C_RESET); scanf("%19s", date);
            salesReportByPeriod(date, "Daily");
        } else if (choice == 2) {
            char month[10]; printf(FG_YELLOW "Enter month (YYYY-MM): " C_RESET); scanf("%9s", month);
            salesReportByPeriod(month, "Monthly");
        } else if (choice == 3) productWiseReport();
        else if (choice == 4) customerSpendingReport();
        else if (choice == 5) {
            char start[20], end[20];
            printf(FG_YELLOW "Start date (YYYY-MM-DD): " C_RESET); scanf("%19s", start);
            printf(FG_YELLOW "End date   (YYYY-MM-DD): " C_RESET); scanf("%19s", end);
            totalIncomeByRange(start, end);
        } else if (choice == 6) break;
        else printf("%s Invalid choice!\n", TAG_ERR);
    }
}

/* ====================== Screens ======================= */
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
        printf("1. Add Item\n2. Display Bill\n3. Finalize Bill\n4. Back\n");
        printf(FG_YELLOW "Choice: " C_RESET);
        if (scanf("%d",&ch)!=1){ while(getchar()!='\n'); continue; }
        if(ch==1){ addItem(); printf(FG_MAGENTA "Press Enter..." C_RESET); getchar(); getchar(); }
        else if(ch==2){ displayBill(); printf(FG_MAGENTA "Press Enter..." C_RESET); getchar(); getchar(); }
        else if(ch==3){
            printf(FG_YELLOW "CustomerID: " C_RESET); 
            if (scanf("%d",&id)==1) finalizeBill(id);
            printf(FG_MAGENTA "Press Enter..." C_RESET); getchar(); getchar();
        }
        else if(ch==4) break;
        else { printf("%s Invalid option\n", TAG_ERR); sleep(1); }
    }
}

/* Customer menu */
void customerMenu() {
    int ch;
    while(1){
        printf("1. Add Customer\n2. Search Customer\n3. Update Customer\n4. Fetch Receipts\n5. Back\n");
        printf(FG_YELLOW "Choice: " C_RESET);
        if (scanf("%d",&ch)!=1){ while(getchar()!='\n'); continue; }

        if(ch==1){ addCustomer(); printf(FG_MAGENTA "Press Enter..." C_RESET); getchar(); getchar(); }
        else if(ch==2){ searchCustomer(); printf(FG_MAGENTA "Press Enter..." C_RESET); getchar(); getchar(); }
        else if(ch==3){ updateCustomer(); printf(FG_MAGENTA "Press Enter..." C_RESET); getchar(); getchar(); }
        else if(ch==4){ int id; printf(FG_YELLOW "Enter ID: " C_RESET); scanf("%d",&id); fetchReceiptHistory(id); printf(FG_MAGENTA "Press Enter..." C_RESET); getchar(); getchar(); }
        else if(ch==5) break;
        else { printf("%s Invalid option\n", TAG_ERR); sleep(1); }
    }
}

/* ====================== main ======================= */
int main() {
    enable_ansi_if_windows();

    int ch;
    while(1){
        printf("1. Billing\n2. Customer Management\n3. Reports\n4. Exit\n");
        printf(FG_YELLOW "Choice: " C_RESET);
        if (scanf("%d",&ch)!=1){ while(getchar()!='\n'); continue; }

        if(ch==1) billingMenu();
        else if(ch==2) customerMenu();
        else if(ch==3) reportMenu();
        else if(ch==4) break;
        else { printf("%s Invalid option\n", TAG_ERR); sleep(1); }
    }
    clear_screen(); 
    return 0;
}
