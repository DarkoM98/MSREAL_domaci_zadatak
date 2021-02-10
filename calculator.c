#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

FILE* fp;

 
// A structure to represent a stack
struct StackNode {
    char data;
    struct StackNode* next;
};
 
struct StackNode* newNode(char data)
{
    struct StackNode* stackNode = 
      (struct StackNode*)
      malloc(sizeof(struct StackNode));
    stackNode->data = data;
    stackNode->next = NULL;
    return stackNode;
}

 
int isEmpty(struct StackNode* root)
{
    return !root;
}
 
void push(struct StackNode** root, char data)
{
    struct StackNode* stackNode = newNode(data);
    stackNode->next = *root;
    *root = stackNode;
    printf("%c pushed to stack\n", data);
}
 
int pop(struct StackNode** root)
{
    if (isEmpty(*root))
        return CHAR_MIN;
    struct StackNode* temp = *root;
    *root = (*root)->next;
    char popped = temp->data;
    free(temp);
 
    return popped;
}
 
int peek(struct StackNode* root)
{
    if (isEmpty(root))
        return CHAR_MIN;
    return root->data;
}

#define BUFF_MAX 200

int op_priority(char op){
	if(op == '+' || op == '-') return 0;
	else if(op == '*' || op == '/') return 1;
	else if(op == '(' || op == ')') return 2;
	else printf("%c nije dozvoljna operacija\n", op);
}

int is_it_op(char c){
	if(c == '+' || c == '-' || c == '*' || c == '/' || c == '(' || c== ')') return 1;
	else return 0;
}

int string_to_int(char *c){
	int val;
	sscanf(c, "%d", &val);
	return val;
}

int operacija_nad_registrima(char regx, char regy, char op){
	if(op == '*') op = 'x';
	char buff[30];
	fp = fopen("/dev/alu", "w");
	sprintf(buff, "reg%c %c reg%c\n", regx, op, regy);
	fputs(buff, fp);
	fclose(fp);
}

void set_register(char reg, int val){
	fp = fopen("/dev/alu", "w");
	char buff[30];
	sprintf(buff, "reg%c=0x%x\n", reg, val);
	//printf("%s\n", buff);
	fputs(buff, fp);
	fclose(fp);
}

int read_result(){
	char buff[30];
	fp = fopen("/dev/alu", "r");
	int b, c;
	fscanf(fp, "%d %d", &b, &c);
	fclose(fp);
	//printf("carry je %d", c);
	if(c == 1 && b != 0){
		printf("Doslo je do prekoracenja opsega\nRezultat je neispravan\n");
		//goto input;
		return -1;
	}
	return b;
}

void perform_op(char op){
	if(op == '*') op = 'x';
	operacija_nad_registrima('a', 'b',op);
}



int main(){
	char str[30];
	fp = fopen("/dev/alu", "w");
	sprintf(str, "format=dec\n");
	fputs(str, fp);
	fclose(fp);

	char buff[BUFF_MAX];
	char RPN[2*BUFF_MAX];
	struct StackNode* root = NULL;
	int res = 0;

	int i = 0;

	//push(&root, '-');

	while(strcmp(buff, "exit")){
	int rpn_pos = 0;
	for(i=0;i<2*BUFF_MAX;i++) RPN[i] = ' '; //init RPN
input:
	rpn_pos = 0;
	for(i=0;i<2*BUFF_MAX;i++) RPN[i] = ' ';
	printf("Unesi izraz: ");
	scanf("%s", buff);
	//printf("%d", buff[0]);
	
	if(!strcmp(buff, "exit")) return 0;

	if(buff[0] == '\0') goto input;
	int usao_u_zagradu = 0;
	i=0;
	while(buff[i]!='\0'){
		if(!is_it_op(buff[i])){
			RPN[rpn_pos] = buff[i];
			rpn_pos++;
		}else{
			if(buff[i] != '('){
			if(RPN[rpn_pos-1] != ' '){
							RPN[rpn_pos] = ' ';
							rpn_pos++;
							}
		}

			if(usao_u_zagradu == 0){

				if(isEmpty(root) || op_priority(peek(root)) < op_priority(buff[i])){
					push(&root, buff[i]);
					if(buff[i] == '(')
					{
						usao_u_zagradu++;
						//printf("usao u zagradu\n\n\n");
					}	

					//printf("Usao");
				}else{

					while(!isEmpty(root)){
						char temp = pop(&root);
						if(temp != '(' && temp != ')'){
						RPN[rpn_pos] = temp;
						rpn_pos++;

						//printf("temp je %c\n\n\n", temp);
						if(RPN[rpn_pos-1] != ' '){
							RPN[rpn_pos] = ' ';
							rpn_pos++;
							}
						}
						//printf("pooped %c\n", pop(&root));			
					}

					push(&root, buff[i]);
				}
			}else{
				if(peek(root) == '(' || op_priority(peek(root)) < op_priority(buff[i])){
					push(&root, buff[i]);
					if(buff[i] == '(')
					{
						usao_u_zagradu++;
						//printf("usao u zagradu\n\n\n");
					}

					if(buff[i] == ')')
					{
						usao_u_zagradu--;
						pop(&root);

						while(peek(root) != '('){
							RPN[rpn_pos] = pop(&root);
							rpn_pos++;

							//printf("temp je %c\n\n\n", temp);

							if(RPN[rpn_pos-1] != ' '){
							RPN[rpn_pos] = ' ';
							rpn_pos++;
							}
						}
						pop(&root);
						//printf("izasao iz zagrade\n");
					}

					//printf("Usao");
				}else{

					while(peek(root) != '('){
						char temp = pop(&root);
						if(temp != ')'){
							RPN[rpn_pos] = temp;
							rpn_pos++;

							//printf("usaso\n");

							if(RPN[rpn_pos-1] != ' '){
							RPN[rpn_pos] = ' ';
							rpn_pos++;
							}
						}
						//printf("peek je %c", peek(root));			
					}

					push(&root, buff[i]);
				}
			}
		}
		//if(is_it_op(buff[i])) push(&root, buff[i]);
		i++;
	}
	
	while(!isEmpty(root)){
		char temp = pop(&root);
		//printf("temp: %c\n", temp);

		if(temp != '('){
		if(RPN[rpn_pos-1] != ' '){
							RPN[rpn_pos] = ' ';
							rpn_pos++;
							}


		RPN[rpn_pos] = temp;
		rpn_pos++;
	}else rpn_pos--;
		//printf("pooped %c\n", pop(&root));			
	}

	if(RPN[1] == ' '){//kada je prvi broj jednocifren pravi problem, pa sam dodao 0 pre broja, tako radi
		char temp[2*BUFF_MAX+2];
		sprintf(temp, "0%s", RPN);
		sprintf(RPN, "%s", temp);
		rpn_pos++;
	}

	while(RPN[rpn_pos] == ' ') rpn_pos--;
	rpn_pos++;

	//printf("%s\n", RPN);
	for(i=0;i<rpn_pos;i++) printf("%c", RPN[i]);
	printf("\n");
	i = 0;
//-----------------------------------------------------//
	//--------razresavanja RPN preko steka----------//
	char val1_str[10]={' ',' ',' '}; 
	char val2_str[10]={' ',' ',' '};
/*
		while(RPN[i]!= ' '){
			val_str[i]=RPN[i];
			i++;
		}
*/

		int stack_pos = 0;
		i = 0;
		while(stack_pos < rpn_pos){
		while(!is_it_op(RPN[stack_pos])){
			push(&root, RPN[stack_pos]);
			stack_pos++;
		}
		push(&root, RPN[stack_pos]);
		stack_pos++;

		i = 0;
		char op = pop(&root);
		pop(&root);
		//printf("op je %c\n", op);
		while(1){
			char temp = pop(&root);
			val2_str[3-i]=temp;
			i++;
			//printf("val2 od %d je %c\n", 4-i, temp);
			if(temp == ' ' || isEmpty(root)) break;
		}

		i = 0;
		while(1){
			char temp = pop(&root);
			val1_str[3-i]=temp;
			i++;
			//printf("%c\n", temp);
			if(temp == ' ' || isEmpty(root)) break;
		}
		int val1 = string_to_int(val1_str);
		int val2 = string_to_int(val2_str);
		//printf("\nval1: %s\nval2: %s\n", val1_str, val2_str);
		set_register('a', val1);
		set_register('b', val2);
		for(i=0;i<5;i++){
			val1_str[i] = ' ';
			val2_str[i] = ' ';
		}
		
		perform_op(op);
		res = read_result();
		if(res == -1) goto input;
		//printf("val1 %d\nval2 %d\n", val1, val2);

		//res = perform_op(val1, val2, op);
		//printf("%d %c %d = %d\n\n", val1,op,val2,res);
		//push(&root, res);
		//printf("res je %d\n", res);
		char res_str[4] = {' ', ' ', ' ', ' '};
		sprintf(res_str, "%d", res);
		//strcat(res_str, " ");
		i = 0;
		push(&root, ' ');
		for(i = 0; i<4;i++){
			if(res_str[i] == '\0') break;
			push(&root, res_str[i]);
		}
}
		//while(val2_str[i] != ' '){
		//	val2_str[4-i] = pop(&root);
		//	i++;
		//}

		//printf("%d\n", string_to_int(val1_str));

		while(!isEmpty(root)){
					pop(&root);
					//printf("pooped %c\n", pop(&root));			
				}

		if(strcmp(buff, "exit")) printf("Rezultat je: %d\n\n", res);
	}



	return 0;

}
