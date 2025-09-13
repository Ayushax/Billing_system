#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "report.h"

#define RECEIPTS_FILE "receipts.txt"

void salesReportByPeriod(const char *prefix, const char *label) {
    FILE *fp = fopen(RECEIPTS_FILE, "r");
    if (!fp) { printf("[ERR] No receipts\n"); return; }
    int receiptID, custID, itemID, qty;
    char date[20]; float amt;
    float total=0; int count=0;
    printf("\n=== %s Report for %s ===\n", label, prefix);
    while(fscanf(fp,"%d %d %d %d %19s %f",&receiptID,&custID,&itemID,&qty,date,&amt)==6){
        if (strncmp(date,prefix,strlen(prefix))==0) {
            printf("Receipt:%d Cust:%d Amount:%.2f Date:%s\n",receiptID,custID,amt,date);
            total+=amt; count++;
        }
    }
    printf("Transactions:%d Total:%.2f\n",count,total);
    fclose(fp);
}

void productWiseReport() {
    FILE *fp=fopen(RECEIPTS_FILE,"r");
    if(!fp){ printf("[ERR] No receipts\n"); return; }
    typedef struct Node { int id; int qty; float amt; struct Node*next;} Node;
    Node*head=NULL;
    int rid,cid,iid,qty; char date[20]; float amt;
    while(fscanf(fp,"%d %d %d %d %19s %f",&rid,&cid,&iid,&qty,date,&amt)==6){
        if(iid<=0) continue;
        Node*p=head; while(p&&p->id!=iid) p=p->next;
        if(!p){p=malloc(sizeof(Node)); p->id=iid;p->qty=0;p->amt=0;p->next=head;head=p;}
        p->qty+=qty; p->amt+=amt;
    }
    printf("\n=== Product-wise Report ===\n");
    for(Node*p=head;p;p=p->next)
        printf("Item:%d Qty:%d Sales:%.2f\n",p->id,p->qty,p->amt);
    fclose(fp);
}

void customerSpendingReport() {
    FILE *fp=fopen(RECEIPTS_FILE,"r");
    if(!fp){ printf("[ERR] No receipts\n"); return; }
    typedef struct Node { int id; int qty; float amt; struct Node*next;} Node;
    Node*head=NULL;
    int rid,cid,iid,qty; char date[20]; float amt;
    while(fscanf(fp,"%d %d %d %d %19s %f",&rid,&cid,&iid,&qty,date,&amt)==6){
        Node*p=head; while(p&&p->id!=cid) p=p->next;
        if(!p){p=malloc(sizeof(Node)); p->id=cid;p->qty=0;p->amt=0;p->next=head;head=p;}
        if(iid>0) p->qty+=qty;
        p->amt+=amt;
    }
    printf("\n=== Customer-wise Report ===\n");
    for(Node*p=head;p;p=p->next)
        printf("Customer:%d Purchases:%d Spent:%.2f\n",p->id,p->qty,p->amt);
    fclose(fp);
}

void totalIncomeByRange(const char *startDate, const char *endDate) {
    FILE *fp=fopen(RECEIPTS_FILE,"r");
    if(!fp){ printf("[ERR] No receipts\n"); return; }
    int rid,cid,iid,qty; char date[20]; float amt,total=0;
    while(fscanf(fp,"%d %d %d %d %19s %f",&rid,&cid,&iid,&qty,date,&amt)==6){
        if(strcmp(date,startDate)>=0 && strcmp(date,endDate)<=0) total+=amt;
    }
    printf("\n=== Income from %s to %s ===\n",startDate,endDate);
    printf("NET Income: %.2f\n",total);
    fclose(fp);
}

void reportMenu() {
    int ch;
    while(1){
        printf("\n===== Reports Menu =====\n");
        printf("1. Daily Sales Report\n2. Monthly Sales Report\n3. Product-wise Sales\n4. Customer Spending\n5. Income by Range\n6. Back\nChoice: ");
        if(scanf("%d",&ch)!=1){while(getchar()!='\n');continue;}
        if(ch==1){ char d[20]; printf("Enter date (YYYY-MM-DD): "); scanf("%19s",d); salesReportByPeriod(d,"Daily"); }
        else if(ch==2){ char m[10]; printf("Enter month (YYYY-MM): "); scanf("%9s",m); salesReportByPeriod(m,"Monthly"); }
        else if(ch==3) productWiseReport();
        else if(ch==4) customerSpendingReport();
        else if(ch==5){ char s[20],e[20]; printf("Start date: "); scanf("%19s",s); printf("End date: "); scanf("%19s",e); totalIncomeByRange(s,e);}
        else if(ch==6) break;
    }
}
