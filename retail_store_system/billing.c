#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "billing.h"
#include "utils.h"

#define ITEMS_FILE     "items.txt"
#define RECEIPTS_FILE  "receipts.txt"

item *head=NULL;
int globalReceiptID=1;

/* Find item by product name */
int findItemByName(const char *searchName, int *outCode, float *outPrice) {
    FILE *fp=fopen(ITEMS_FILE,"r");
    if(!fp) return 0;
    int fileCode; char name[50]; float price;
    while(fscanf(fp," %d , %49[^,] , %f",&fileCode,name,&price)==3) {
        if(strcasecmp(name, searchName)==0) {  
            if(outCode) *outCode = fileCode;
            if(outPrice) *outPrice = price;
            fclose(fp);
            return 1;
        }
        int c; while((c=fgetc(fp))!='\n' && c!=EOF){} 
    }
    fclose(fp); return 0;
}

/* Create new cart node */
item* createNode() {
    item* newnode=(item*)malloc(sizeof(item));
    if(!newnode) return NULL;
    printf("Enter product name: ");
    if (scanf("%s",newnode->name)!=1){ free(newnode); return NULL; }

    if(!findItemByName(newnode->name,&newnode->product_code,&newnode->price)){
        printf("[ERR] Product not found\n");
        free(newnode); return NULL;
    }
    printf("Product: %s  Price: %.2f\n", newnode->name,newnode->price);
    printf("Enter qty: ");
    if (scanf("%d",&newnode->quantity)!=1){ free(newnode); return NULL; }

    newnode->total=newnode->price*newnode->quantity;
    newnode->next=NULL; return newnode;
}

void addItem() {
    item* n=createNode(); if(!n) return;
    if(!head) head=n;
    else {item* t=head; while(t->next) t=t->next; t->next=n;}
    printf("[OK] Item added to cart\n");
}

void print_bill_sidebar(int start_row) {
    int col = 60, boxWidth = 40;
    move_cursor(start_row, col);
    printf("+%.*s+\n", boxWidth-2, "--------------------------------------");
    move_cursor(start_row+1, col);
    printf("| CURRENT BILL %*s|\n", boxWidth-14,"");
    move_cursor(start_row+2, col);
    printf("+%.*s+\n", boxWidth-2, "--------------------------------------");

    item* t=head; int row=start_row+3;
    float g=0; int shown=0;
    while(t && shown<10) {
        char line[100];
        snprintf(line,sizeof(line),"%s x%d = %.2f",t->name,t->quantity,t->total);
        if((int)strlen(line)>boxWidth-4) line[boxWidth-4]=0;
        move_cursor(row++,col);
        printf("| %-*s |\n",boxWidth-4,line);
        g+=t->total; t=t->next; shown++;
    }
    for(; shown<10; shown++,row++){
        move_cursor(row,col);
        printf("| %-*s |\n",boxWidth-4,"");
    }

    float discount = (g >= 1000.0f) ? g * 0.05f : 0.0f;
    float payable  = g - discount;

    move_cursor(row,col);
    printf("+%.*s+\n", boxWidth-2, "--------------------------------------");
    move_cursor(row+1,col);
    printf("| Subtotal: %-18.2f |\n", g);
    move_cursor(row+2,col);
    printf("| Disc(5%%): %-17.2f |\n", discount);
    move_cursor(row+3,col);
    printf("| Payable:  %-17.2f |\n", payable);
    move_cursor(row+4,col);
    printf("+%.*s+\n", boxWidth-2, "--------------------------------------");
}

void displayBill() {
    if(!head){printf("[ERR] Cart is empty\n");return;}
    float subtotal=0;
    printf("\n%-20s %-5s %-9s %-9s\n","Product","Qty","Price","Total");
    printf("------------------------------------------------------------\n");
    item*t=head;
    while(t){
        printf("%-20s %-5d %-9.2f %-9.2f\n",t->name,t->quantity,t->price,t->total);
        subtotal+=t->total; t=t->next;
    }
    float discount = (subtotal >= 1000.0f) ? subtotal * 0.05f : 0.0f;
    float payable  = subtotal - discount;
    printf("------------------------------------------------------------\n");
    printf("Subtotal : %.2f\n", subtotal);
    printf("Discount : %.2f%s\n", discount, discount>0 ? " (5%)" : "");
    printf("Payable  : %.2f\n", payable);
}

void freeCart() {
    item *t;
    while(head){ t=head; head=head->next; free(t); }
}

void finalizeBill(int custID) {
    if(!head){printf("[ERR] No items to bill\n");return;}
    FILE *fp=fopen(RECEIPTS_FILE,"a");
    if(!fp){ printf("[ERR] Could not open %s\n", RECEIPTS_FILE); return; }

    char date[20]; getDate(date);
    float subtotal=0.0f;

    for(item*t=head; t; t=t->next){
        fprintf(fp,"%d %d %d %d %s %.2f\n",
                globalReceiptID,custID,t->product_code,t->quantity,date,t->total);
        subtotal += t->total;
    }
    float discount = (subtotal >= 1000.0f) ? subtotal * 0.05f : 0.0f;
    float payable  = subtotal - discount;
    if (discount > 0.0f) {
        fprintf(fp,"%d %d %d %d %s %.2f\n",
                globalReceiptID, custID, -1, 0, date, -discount);
    }
    fprintf(fp,"%d %d %d %d %s %.2f\n",
            globalReceiptID, custID, -2, 0, date, payable);

    fclose(fp);
    printf("[OK] Bill saved (ReceiptID: %d)\n", globalReceiptID);
    globalReceiptID++;
    freeCart();
}
