#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#define MAXDATA 1000
#define MAXATTB 100

struct data{
	int nume[MAXATTB], cate[MAXATTB];
	int class;
};

typedef struct node{
	struct node *lchild, *rchild;
	struct data d[MAXDATA];
	int numeidx, cateidx; // not used at leaf node
	int numeval;
	int class; // only used at leaf node
} node;

node *head;
int numenum, catenum;
int numeflag[MAXATTB] = {0, }; // if attb used: 0 -> 1
int cateflag[MAXATTB] = {0, };

node *Initnode();
void Inithead(int *);
void Settree(node **, int);
double Ignume(struct data[], int [], int, int);
int Setdata(int [], int []);
void Printtree(node *);

int main(){
	FILE *finput, *foutput; 

	int datanum;
	Inithead(&datanum); // head node

	Settree(&head, datanum);
	//Printtree(head);

	finput = fopen("test.txt", "r");
	foutput = fopen("output.txt", "w");

	fscanf(finput, "%d %d %d", &datanum, &numenum, &catenum);
	for(int i = 0;i < datanum;i++){
		//printf("data [%d]\n", i);
		int cate[MAXATTB], nume[MAXATTB];
		for(int j = 0;j < numenum;j++){
			fscanf(finput, "%d", &nume[j]);
			fprintf(foutput, "%d ", nume[j]);
		}
		for(int j = 0;j < catenum;j++){
			fscanf(finput, "%d", &cate[j]);
			fprintf(foutput, "%d ", cate[j]);
		}
		fprintf(foutput, "%d\n", Setdata(nume, cate));
	}

	fclose(finput); fclose(foutput);
	return 0;
}

void Printtree(node *curr){
	if(!curr){
		printf("NULL\n");
		return;
	}
	
	if(curr->numeidx > -1){ // nume
		int attb = curr->numeval;
		printf("NUME %d\n", attb);
	}
	else if(curr->cateidx > -1){ // cate
		printf("CATE %d\n", curr->cateidx);
	}
	else{ // leaf node -> class set
		printf("CLASS %d\n", curr->class);
	}

	Printtree(curr->lchild);
	Printtree(curr->rchild);
}

int Setdata(int nume[], int cate[]){
	node *curr = head;
	while(curr){
		if(curr->numeidx > -1){ // nume
			int attb = curr->numeval;
			if(nume[curr->numeidx] <= attb)
				curr = curr->lchild;
			else curr = curr->rchild;
		}
		else if(curr->cateidx > -1){ // cate
			if(cate[curr->cateidx])
				curr = curr->lchild;
			else curr = curr->rchild;
		}
		else{ // leaf node -> class set
			return (curr->class);
		}
	}
	printf("Setdata Error\n"); exit(1);
}

void Settree(node **curr, int datanum){
	int i, j, k, least, ldatanum, rdatanum, attbidx, jidx, leafflag = 0, nflag = 0;
	double ig, numeig = -1000;

	for(i = 0;i < numenum;i++) if(numeflag[i] == 0) break;

	if(i == numenum){ // numeflag is full
		for(k = 0;k < catenum;k++) if(cateflag[k] == 0) break;
		
		if(k == catenum){ 
			// cateflag is full -> no attb -> leaf node
			int cnt0 = 0, cnt1 = 0;
			for(j = 0;j < datanum;j++){
				switch((*curr)->d[j].class){
					case 0: cnt0++; break;
					case 1: cnt1++; break;
					default: printf("Class Error\n"); exit(1);
				}
			}
			if(cnt0 >= cnt1) (*curr)->class = 0;
			else (*curr)->class = 1;
			return; // leaf node-> end
		}
	}

	int numeidx[MAXDATA]; // sort index
	for(j = 0;j < datanum;j++) numeidx[j] = j;
	ig = Ignume((*curr)->d, numeidx, 0, datanum);

	/* numeric attb */
	for(i = 0;i < numenum;i++){
		if(numeflag[i]) continue; // already used

		/* sort */
		// numeidx = {0, 1, 2, ...}
		// max_j = datanum
		for(j = 0;j < datanum;j++) numeidx[j] = j;
		
		for(j = 0;j < datanum - 1;j++){
			least = j;
			for(k = j + 1;k < datanum;k++){
				if((*curr)->d[numeidx[k]].nume[i] < (*curr)->d[numeidx[least]].nume[i])
					least = k;
			}
			if(least != j){ // swap least - j
				int tmp = numeidx[j];
				numeidx[j] = numeidx[least];
				numeidx[least] = tmp;
			}	
		}

		/* ig */
		jidx = -1;
		double temp, bigig = -10000;
		double left = 0, right = 0;
		for(j = 0;j < datanum - 1;j++) { // set bigig
			// [0 ~ j]  [(j + 1) ~ (datanum - 1)] <- use index of numeidx[]

			// IG(class, j)
			left = Ignume((*curr)->d, numeidx, 0, j + 1) * (j + 1) / datanum;
			right = Ignume((*curr)->d, numeidx, j + 1, datanum) * (datanum - j - 1) / datanum;
			temp = ig - (left + right);

			if(j == 0) {bigig = temp; jidx = j;}
			else if(temp > bigig) {bigig = temp; jidx = j;}

			if(bigig == ig) {// leaf node
				leafflag = 1; break;
			}
		}

		if(leafflag){ // leaf node -> STOP
			// set class
			node *lchild = Initnode();
			node *rchild = Initnode();

			lchild->class = (*curr)->d[numeidx[0]].class;
			rchild->class = (*curr)->d[numeidx[jidx + 1]].class;

			(*curr)->lchild = lchild;
			(*curr)->rchild = rchild;

			(*curr)->numeidx = i;
			(*curr)->numeval = (*curr)->d[numeidx[jidx]].nume[i];

			numeflag[i] = 1;
			return;
		}

		/* Set child node (can be changed) */
		if(bigig > numeig){
			int cnt0 = 0, cnt1 = 0;
			nflag = 1; numeig = bigig; attbidx = i;
			(*curr)->numeval = (*curr)->d[numeidx[jidx]].nume[i];
			ldatanum = jidx + 1; rdatanum = datanum - jidx - 1;

			node *lchild = Initnode();
			node *rchild = Initnode();

			// if child node is leaf node
			for(j = 0;j < jidx + 1;j++){
				if((*curr)->d[numeidx[j]].class) cnt1++;
				else cnt0++;
			}
			if(cnt0 == 0) lchild->class = 1;
			else if(cnt1 == 0) lchild->class = 0;

			cnt0 = cnt1 = 0;
			for(j = jidx + 1;j < datanum;j++){
				if((*curr)->d[numeidx[j]].class) cnt1++;
				else cnt0++;
			}
			if(cnt0 == 0) rchild->class = 1;
			else if(cnt1 == 0) rchild->class = 0;

			for(k = j = 0;j < jidx + 1;j++, k++){
				lchild->d[k].class = (*curr)->d[numeidx[j]].class;
				
				for(int w = 0;w < numenum;w++)
					lchild->d[k].nume[w] = (*curr)->d[numeidx[j]].nume[w];
				for(int w = 0;w < catenum;w++)
					lchild->d[k].cate[w] = (*curr)->d[numeidx[j]].cate[w];
			}

			for(k = 0, j = jidx + 1;j < datanum;j++, k++){
				rchild->d[k].class = (*curr)->d[numeidx[j]].class;
				
				for(int w = 0;w < numenum;w++)
					rchild->d[k].nume[w] = (*curr)->d[numeidx[j]].nume[w];
				for(int w = 0;w < catenum;w++)
					rchild->d[k].cate[w] = (*curr)->d[numeidx[j]].cate[w];
			}

			if((*curr)->lchild) free((*curr)->lchild);
			if((*curr)->rchild) free((*curr)->rchild);
			
			(*curr)->lchild = lchild;
			(*curr)->rchild = rchild;

		}
	}
	numeflag[attbidx] = 1;

	/* categoric attb */
	for(j = 0;j < datanum;j++) numeidx[j] = j;
	
	for(i = 0;i < catenum;i++){
		if(cateflag[i]) continue;

		/* ig */
		double temp; 
		int lcateidx[MAXATTB], rcateidx[MAXATTB]; // cateidx[k]
		int lidx = 0, ridx = 0;
		// divide by cate[i] (0 or 1)
		for(j = 0;j < datanum;j++){
			switch((*curr)->d[j].cate[i]){
				case 1:
					lcateidx[lidx++] = j;
					break;
				case 0:
					rcateidx[ridx++] = j;
					break;
				default:
					printf("Cate value error\n");
					exit(1);
			}
		}

		if(lidx == 0) temp = 0;
		else temp = Ignume((*curr)->d, lcateidx, 0, lidx) * lidx / datanum;
		if(ridx != 0) temp += Ignume((*curr)->d, rcateidx, 0, ridx) * ridx / datanum;
		temp = ig - temp;

		if(ig == temp){ // leafflag = 1 -> leaf node -> STOP
			// set class
			if(nflag) numeflag[attbidx] = 0;
			cateflag[i] = 1;

			node *lchild = Initnode();
			lchild->class = (*curr)->d[lcateidx[0]].class;
			node *rchild = Initnode();
			rchild->class = (*curr)->d[rcateidx[0]].class;

			(*curr)->lchild = lchild;
			(*curr)->rchild = rchild;

			(*curr)->cateidx = i;
			return;
		}

		/* Set child node */
		if(temp > numeig){
			if(nflag){
				numeflag[attbidx] = 0; // not in numeric
				nflag = 0;
			}
			numeig = temp; attbidx = i;
			ldatanum = lidx; rdatanum = ridx;

			node *lchild = Initnode();
			node *rchild = Initnode();
			
			// if child node is leaf node
			int cnt0 = 0, cnt1 = 0;
			for(j = 0;j < lidx;j++){
				if((*curr)->d[lcateidx[j]].class) cnt1++;
				else cnt0++;
			}
			if(cnt0 == 0) lchild->class = 1;
			else if(cnt1 == 0) lchild->class = 0;

			cnt0 = cnt1 = 0;
			for(j = 0;j < ridx;j++){
				if((*curr)->d[rcateidx[j]].class) cnt1++;
				else cnt0++;
			}
			if(cnt0 == 0) rchild->class = 1;
			else if(cnt1 == 0) rchild->class = 0;

			for(j = 0;j < lidx;j++){ // j: data idx
				lchild->d[j].class = (*curr)->d[lcateidx[j]].class;

				for(int w = 0;w < numenum;w++)
					lchild->d[j].nume[w] = (*curr)->d[lcateidx[j]].nume[w];
				for(int w = 0;w < catenum;w++)
					lchild->d[j].cate[w] = (*curr)->d[lcateidx[j]].cate[w];
			}

			for(j = 0;j < ridx;j++){
				rchild->d[j].class = (*curr)->d[rcateidx[j]].class;

				for(int w = 0;w < numenum;w++)
					rchild->d[j].nume[w] = (*curr)->d[rcateidx[j]].nume[w];
				for(int w = 0;w < catenum;w++)
					rchild->d[j].cate[w] = (*curr)->d[rcateidx[j]].cate[w];
			}

			if((*curr)->lchild) free((*curr)->lchild);
			if((*curr)->rchild) free((*curr)->rchild);

			(*curr)->lchild = lchild;
			(*curr)->rchild = rchild;
		}
	}

	/* set final child node */
	if(nflag == 0){ // by cate
		cateflag[attbidx] = 1;
		(*curr)->cateidx = attbidx;
	}
	else { // by nume
		(*curr)->numeidx = attbidx;
	}

	if((*curr)->lchild->class == -1) Settree(&((*curr)->lchild), ldatanum);
	if((*curr)->rchild->class == -1) Settree(&((*curr)->rchild), rdatanum);
	return;
}

double Ignume(struct data d[], int numeidx[], int x, int y){
	// d[x] ~ d[y - 1]
	double ig = 0; int i;
	int cnt0 = 0, cnt1 = 0;
	
	for(i = x;i < y;i++){
		switch(d[numeidx[i]].class){
			case 0:
				cnt0++;
				break;
			case 1:
				cnt1++;
				break;
			default:
				printf("Class Error\n");
				exit(1);
		}
	}
	if(cnt0 == 0 || cnt1 == 0)
		return 0;

	double temp = (double)cnt0 / ((double)cnt0 + (double)cnt1);
	ig = -temp * log2(temp);
	temp = (double)cnt1 / ((double)cnt0 + (double)cnt1);
	ig += -temp * log2(temp);
	return ig;
}

node *Initnode(){
	node *newnode = (node *)malloc(sizeof(node));
	newnode->lchild = newnode->rchild = NULL;
	newnode->numeidx = newnode->cateidx = newnode->class = -1;
	return newnode;
}

void Inithead(int *datanum){
	FILE *ftrain;
	if((ftrain = fopen("train.txt", "r")) == NULL){
		printf("Fopen error\n"); exit(1);
	}

	/* init on head node (head has all data) */
	head = (node *)malloc(sizeof(node));
	head->lchild = head->rchild = NULL;
	head->numeidx = head->cateidx = head->class = -1;

	int i, j;
	fscanf(ftrain, "%d %d %d", datanum, &numenum, &catenum);

	for(i = 0;i < *datanum;i++){
		for(j = 0;j < numenum;j++)
			fscanf(ftrain, "%d", &(head->d[i].nume[j]));

		for(j = 0;j < catenum;j++)
			fscanf(ftrain, "%d", &(head->d[i].cate[j]));

		fscanf(ftrain, "%d", &(head->d[i].class));
	}

	fclose(ftrain);
}
