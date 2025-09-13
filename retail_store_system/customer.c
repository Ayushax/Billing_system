#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "customer.h"

#define CUSTOMERS_FILE "customers.txt"
#define RECEIPTS_FILE  "receipts.txt"

void addCustomer() {
    FILE *fp=fopen(CUSTOMERS_FILE,"a");
    if(!fp){printf("[ERR] Unable to open %s\n", CUSTOMERS_FILE);return;}
    Customer c;
    printf("Enter ID: "); scanf("%d",&c.custID); getchar();
    printf("Enter Name: "); fgets(c.name,50,stdin); c.name[strcspn(c.name,"\n")]=0;
    printf("Enter Phone: "); scanf("%14s",c.phone);
    printf("Enter Email: "); scanf("%49s",c.email); getchar();
    printf("Enter Address: "); fgets(c.address,100,stdin); c.address[strcspn(c.address,"\n")]=0;
    fprintf(fp,"%d %s %s %s %s\n",c.custID,c.name,c.phone,c.email,c.address);
    fclose(fp);
    printf("[OK] Customer added\n");
}

void searchCustomer() {
    FILE *fp=fopen(CUSTOMERS_FILE,"r");
    if(!fp){printf("[ERR] Open %s failed\n", CUSTOMERS_FILE);return;}
    char key[50]; printf("Enter Name/Phone: "); scanf(" %[^\n]",key);
    int id; char name[50],phone[15],email[50],addr[100]; int found=0;
    while(fscanf(fp,"%d %49s %14s %49s %99s",&id,name,phone,email,addr)==5){
        if(strstr(name,key)||strcmp(phone,key)==0){
            printf("[INFO] ID:%d  Name:%s  Phone:%s  Email:%s  Addr:%s\n",id,name,phone,email,addr);
            found=1; break;
        }
    }
    if(found==0) printf("[ERR] Not found\n");
    fclose(fp);
}

void updateCustomer() {
    FILE *fp = fopen(CUSTOMERS_FILE, "r");
    if (!fp) { printf("[ERR] Open %s failed!\n", CUSTOMERS_FILE); return; }
    Customer customers[100]; int count=0;
    while (fscanf(fp, "%d %49s %14s %49s %99s",
                  &customers[count].custID,
                  customers[count].name,
                  customers[count].phone,
                  customers[count].email,
                  customers[count].address) == 5) {
        count++;
    }
    fclose(fp);
    int id; printf("Enter Customer ID to update: "); scanf("%d", &id);
    int found=0;
    for (int i = 0; i < count; i++) {
        if (customers[i].custID == id) {
            printf("Updating: %s\n", customers[i].name);
            printf("New Phone: "); scanf("%14s", customers[i].phone);
            printf("New Email: "); scanf("%49s", customers[i].email); getchar();
            printf("New Address: "); fgets(customers[i].address, 100, stdin);
            customers[i].address[strcspn(customers[i].address, "\n")] = 0;
            found=1; break;
        }
    }
    if (!found) { printf("[ERR] Customer ID not found!\n"); return; }
    fp = fopen(CUSTOMERS_FILE, "w");
    for (int i = 0; i < count; i++) {
        fprintf(fp, "%d %s %s %s %s\n",
                customers[i].custID,
                customers[i].name,
                customers[i].phone,
                customers[i].email,
                customers[i].address);
    }
    fclose(fp);
    printf("[OK] Customer updated\n");
}

void fetchReceiptHistory(int cid) {
    FILE *fp=fopen(RECEIPTS_FILE,"r");
    if(!fp){printf("[INFO] No receipts yet\n");return;}
    int rid, cust, item, qty; char date[20]; float amt; int found=0;
    while(fscanf(fp,"%d %d %d %d %19s %f",&rid,&cust,&item,&qty,date,&amt)==6){
        if(cust==cid){
            if (item == -1) printf("Receipt:%d DISCOUNT Amt:%.2f Date:%s\n", rid, amt, date);
            else if (item == -2) printf("Receipt:%d TOTAL    Amt:%.2f Date:%s\n", rid, amt, date);
            else printf("Receipt:%d Item:%d Qty:%d Amt:%.2f Date:%s\n",rid,item,qty,amt,date);
            found=1;
        }
    }
    if(!found) printf("[INFO] No receipts for this customer\n");
    fclose(fp);
}
