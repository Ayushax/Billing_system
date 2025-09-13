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
void reportMenu() {
    int ch;
    while(1){
        clear_screen(); print_left_header(" - Reports"); print_bill_sidebar(2);
        move_cursor(6,1);
        printf("1.Show receipts\n2.Back\nChoice: "); scanf("%d",&ch);
        if(ch==1){
            FILE *fp=fopen(RECEIPTS_FILE,"r");
            if(!fp) printf("No receipts\n");
            else {char l[256]; while(fgets(l,sizeof(l),fp)) printf("%s",l); fclose(fp);}
            getchar(); getchar();
        } else if(ch==2) break;
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
        printf("1.Add Customer\n2.Search Customer\n3.Fetch Receipts\n4.Back\nChoice: ");
        scanf("%d",&ch);
        if(ch==1){addCustomer(); getchar(); getchar();}
        else if(ch==2){searchCustomer(); getchar(); getchar();}
        else if(ch==3){int id; printf("Enter ID: "); scanf("%d",&id); fetchReceiptHistory(id); getchar(); getchar();}
        else if(ch==4) break;
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
