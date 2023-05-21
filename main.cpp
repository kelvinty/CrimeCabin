#include <stdio.h>
#include <stdlib.h>
#include <graphics.h>
#include <windows.h>
#include <locale.h>
#include <time.h>
#include <conio.h>
#include <math.h>
#include <string.h>

#define QtdCenas  82

//STRUCTS

struct TMouse{
	int x;
	int y;
	int largura;
	int altura;
};

typedef struct botao{
	char *nome;
	int x;
	int y;
	int largura;
	int altura;
	bool clicado;
	void* imagem;
	void* mascara;
} Botao;

typedef struct vetor_botoes{
	int capacidade;
	int tamanho;
	Botao *botoes;
} BotoesVetor; 


typedef struct item{
	int id;
	char *nome;
	void *imagem;
	void *mascara;
	int x;
	int y;
	int largura;
	int altura;
} Item;

typedef struct vetor_itens{
	int capacidade;
	int tamanho;
	Item *itens;
} ItensVetor; 

typedef struct final {
	int id;
	ItensVetor *itens;
	char *descricao;
	char *historia; 
} Final;

typedef struct vetor_finais {
	int capacidade;
	int tamanho;
	Final *finais;
} FinaisVetor;

struct TInventario {
	int maxItens;
	int qtdItens;
	ItensVetor *itens;
	int x;
	int y;
};

typedef struct saida {
	int id;
	char* nome;
	int x;
	int y;
	int largura;
	int altura;
	FinaisVetor *finais;
}Saida;

struct TCamera {
	int id;
	int qtdItens;
	void *imagem;
	ItensVetor *itens;
	Saida *saida;
};

//VARIAVEIS GLOBAIS
int pg = 1;
int last_time = clock();

//Loops do jogo;
int comecaJogo();
int Tutorial();
int Menu();
int Conclusao(Final final);
int Animacao();

//Funções do jogo

void* load_image(const char *endereco, int largura, int altura, int x, int y){
	int aux = imagesize(x,y,largura,altura);
	void*img = malloc(aux);
	readimagefile(endereco, x, y, largura, altura);
	getimage(x, y, largura, altura, img);
	return img;
}

int strlen(char *str)
{
    int total=0;
    while( str[total] != '\0'){
    	total++;
	}    
    return total;
}

void animacao_texto(char *texto,int quebra_linha,int qtd,int pos_x,int pos_y) {
	mciSendString("open .\\Audios\\Teclado.mp3 type MPEGVideo alias Teclado", NULL, 0, 0);
	waveOutSetVolume(0,0x88888888);
	mciSendString("play Teclado repeat", NULL, 0, 0);
	double larguraString = textwidth(texto);
	int alturaString = textheight(texto);
	int qtdLetrasTexto = strlen(texto);
	
	int larguraLetras = larguraString/qtdLetrasTexto;
	
	int larguraTexto = qtdLetrasTexto*larguraLetras;
	
	int qtdLinhas = ceil(larguraString/quebra_linha);
	int qtdLetras = quebra_linha/larguraLetras;
	
	char **linha = (char**)malloc(sizeof(char*)*qtdLinhas);
	char *str = (char*)malloc(sizeof(char)*qtdLinhas);
  	char *newStr = (char*)malloc(sizeof(char)*qtdLetras);
  	
  	for(int k = 0;k < qtdLinhas;k++) {
  		for(int j = 0; j < qtdLetras;){
  			str[j] = texto[j+ (k*qtdLetras)];
			str[j+1] = '\0';
			j++;
		}
		linha[k] = (char*)malloc(sizeof(char)*(qtdLetras+1));
		strcpy(linha[k], str);
	}
	
   	for (int i = 0; i < qtdLinhas;i++) {
   		setfillstyle(1,RGB(0,0,0));
		
   		for(int l = 0; l < qtdLetras;){
   			
   			if(linha[i][l] != '\0'){
   				
				newStr[l] = linha[i][l];
				newStr[l+1] = '\0';
				outtextxy(pos_x ,pos_y + (i*alturaString),newStr);
				delay(60);
				printf("total letras:%d, letra atual:%d\n", qtdLetras, l);
				setactivepage(pg);
				l++;
				
			} else {
				
				mciSendString("stop Teclado", NULL, 0, 0);
				return;
				
			}
		}
        outtextxy(pos_x, pos_y + (i*alturaString), linha[i]);
	}
	
	free(str);

	for(int i = 0; i < qtdLinhas; i++) {
		free(linha[i]);	
	}
	
	free(linha);
	linha = NULL;
	free(newStr);
	
	return;
}

void deleteImage(void *image){
	free(image);
}

Item *criar_item(int id, char *nome,void *imagem, void *mascara, int x, int y, int largura, int altura) {
	Item *item = (Item*)malloc(sizeof(Item));
	
	item->id = id;
	item->nome = nome;
	item->imagem = imagem;
	item->mascara = mascara;
	item->x = x;
	item->y = y;
	item->largura = largura;
	item->altura = altura;
	
	return item;
}



void apaga_item(Item **item_ref) {
	Item *item = *item_ref;
	free(item);
	*item_ref = NULL;
}

ItensVetor *criar_vetor_itens(int capacidade){
	ItensVetor *vec = (ItensVetor*) calloc(1,sizeof(ItensVetor));

	vec->tamanho = 0;
	vec->capacidade = capacidade;
	vec->itens = (Item*) calloc(capacidade,sizeof(Item));

	return vec;
}

void print_vetor_itens(ItensVetor *vec){
	for(int i = 0;i < vec->tamanho;i++){
		printf("item:%s, indice:%d\n",vec->itens[i].nome,i);
	}
}

void apaga_vetor_itens(ItensVetor **vec_ref){
	ItensVetor *vec = *vec_ref;
	
	free(vec->itens);
	free(vec);
	
	*vec_ref = NULL;
}

void append_vetor_itens(ItensVetor *vec, Item *item){
	if(vec->tamanho == vec->capacidade){
		fprintf(stderr,"ERROR in 'append'\n");
		fprintf(stderr,"Vetor cheio'\n");
		exit(EXIT_FAILURE);
	}
	
	vec->itens[vec->tamanho] = *item;

	vec->tamanho++;
}

void compara_vetor_itens(Final final,const ItensVetor *vec_inventario){
	int count = 0;
	
	ItensVetor *vec_final = final.itens;
	
	for(int i = 0;i < vec_final->tamanho;i++){
		Item item_final = vec_final->itens[i];
		for(int j = 0;j < vec_inventario->tamanho; j++){
			Item item_inventario = vec_inventario->itens[j];
			if(item_final.id == item_inventario.id){
				count++;
				printf("item existe no final\n");
			} else {
				printf("item nao existe nesse final\n");
			}
		}
	}
	if(count == 2){
		Conclusao(final);
	} else {
		printf("esse final não combina:%d\n", final.id);
	}
	return;
}

void remove_item_vetor(ItensVetor *vec,Item *item){
	if(vec->tamanho > 0) {
		for(int i = 0;i < vec->tamanho;i++){
			if(vec->itens[i].id == item->id) {
				for(int j = i; j < vec->tamanho -1;j++) {
//					printf("\nachou id:%d | item_id:%d",vec->itens[i].id,item->id);
					vec->itens[j] = vec->itens[j+1];
				}	
				vec->tamanho--;
			}
		}
		print_vetor_itens(vec);
		
	}
}

Botao *criar_botao(char*nome, void *imagem, void *mascara, int x, int y, int largura, int altura) {
	Botao *botao = (Botao*)malloc(sizeof(Botao));
	
	botao->nome = nome;
	botao->imagem = imagem;
	botao->mascara = mascara;
	botao->x = x;
	botao->y = y;
	botao->largura = largura;
	botao->altura = altura;
	
	return botao;
}

BotoesVetor *criar_vetor_botoes(int capacidade){
	BotoesVetor *vec = (BotoesVetor*) calloc(1,sizeof(BotoesVetor));
	
	vec->tamanho = 0;
	vec->capacidade = capacidade;
	vec->botoes = (Botao*) calloc(capacidade,sizeof(Botao));
	
	return vec;
}

void append_vetor_botoes(BotoesVetor *vec, Botao *botao){
	if(vec->tamanho == vec->capacidade){
		fprintf(stderr,"ERROR in 'append'\n");
		fprintf(stderr,"Vetor cheio'\n");
		exit(EXIT_FAILURE);
	}
	
	vec->botoes[vec->tamanho] = *botao;

	vec->tamanho++;
}

void apaga_vetor_botoes(BotoesVetor **vec_ref){
	BotoesVetor *vec = *vec_ref;
	
	free(vec->botoes);
	free(vec);
	
	*vec_ref = NULL;
}


void apaga_botao(Botao **botao_ref) {
	Botao *botao = *botao_ref;
	free(botao);
	*botao_ref = NULL;
}

Final *criar_final(int _id, char *_descricao, char *_historia) {
	Final *final = (Final*)calloc(1,sizeof(Final));
	
	final->id = _id;
	final->itens = NULL;
	final->descricao = _descricao;
	final->historia = _historia;
	
	return final;
}

void apaga_final(Final **final_ref) {
	Final *final = *final_ref;
	free(final);
	*final_ref = NULL;
}

FinaisVetor *criar_vetor_finais(int capacidade){
	FinaisVetor *vec = (FinaisVetor*) calloc(1,sizeof(FinaisVetor));
	
	vec->tamanho = 0;
	vec->capacidade = capacidade;
	vec->finais = (Final*) calloc(capacidade,sizeof(Final));
	
	return vec;
}

void print_vetor_finais(FinaisVetor *vec){
	printf("entrou na funcao");
	printf("tamanho: %d", vec->tamanho);
	for(int i = 0;i < vec->tamanho;i++){
		printf("final:%s, indice:%d\n",vec->finais[i].descricao,i);
	}
}

void apaga_vetor_finais(FinaisVetor **vec_ref){
	FinaisVetor *vec = *vec_ref;
	
	free(vec->finais);
	free(vec);
	
	*vec_ref = NULL;
}

void append_vetor_finais(FinaisVetor *vec, Final *final){
	if(vec->tamanho == vec->capacidade){
		fprintf(stderr,"ERROR in 'append'\n");
		fprintf(stderr,"Vetor cheio'\n");
		exit(EXIT_FAILURE);
	}
	
	vec->finais[vec->tamanho] = *final;

	vec->tamanho++;
}

void remove_final_vetor(FinaisVetor *vec,Final *final){
	if(vec->tamanho > 0) {
		for(int i = 0;i < vec->tamanho;i++){
			if(vec->finais[i].id == final->id) {
				for(int j = i; j < vec->tamanho -1;j++) {
//					printf("\nachou id:%d | item_id:%d",vec->itens[i].id,item->id);
					vec->finais[j] = vec->finais[j+1];
				}	
				vec->tamanho--;
			}
		}
		print_vetor_finais(vec);
	}
}


TMouse *mousePos(){
	TMouse *mouse = (TMouse*)malloc(sizeof(TMouse));
	if(!ismouseclick(WM_LBUTTONDOWN)){
		mouse->x = mousex();
		mouse->y = mousey();
		clearmouseclick(WM_LBUTTONDOWN);
	} 
	if(ismouseclick(WM_LBUTTONDOWN)){
		mouse->x = mousex();
		mouse->y = mousey();
		clearmouseclick(WM_LBUTTONDOWN);
	}
	return mouse;
}


TInventario *criar_inventario(int x, int y) {
	TInventario *inventario = (TInventario*)calloc(1,sizeof(TInventario));
	
//	inventario->maxItens = maxItens;
//	inventario->qtdItens = qtdItens;
	inventario->x = x;
	inventario->y = y;
	inventario->itens = NULL;
	
	return inventario;
}



Saida *criarSaida(int _id,char*_nome,int _x, int _y,int largura, int altura) {
	Saida *saida = (Saida*)calloc(1,sizeof(Saida));
	
	saida->id = _id;
	saida->nome = _nome;
	saida->x = _x;
	saida->y = _y;
	saida->largura = largura;
	saida->altura = altura;
	saida->finais = NULL;
	
	return saida;
}


//CRUD CAMERA

TCamera *criarCamera(int _id,void*_imagem) {
	TCamera *camera = (TCamera*) calloc(1,sizeof(TCamera));
	
	camera->imagem = _imagem;
	camera->id = _id;
	camera->itens = NULL;
	camera->saida = NULL;
	
	return camera;
}

void mostrarCamera(const TCamera *camera) {
	putimage(0,0,camera->imagem,COPY_PUT);
}

void mostrarItensCamera(const TCamera *camera) {
	for(int i =0;i < camera->itens->tamanho;i++) {
		Item item = camera->itens->itens[i]; 
		putimage(item.x, item.y, item.mascara, AND_PUT);
		putimage(item.x, item.y, item.imagem, OR_PUT);
	}
}

void mostrarSaidasCamera(const TCamera *camera) {
	Saida *saida = camera->saida; 
	if(saida != NULL){
//		bar(saida->x,saida->y,saida->largura,saida->altura);
		rectangle(saida->x,saida->y,saida->largura + saida->x,saida->altura+ saida->y);
	}
//	putimage(item.x, item.y, item.mascara, AND_PUT);
//	putimage(item.x, item.y, item.imagem, OR_PUT);
	
}

void mostrarInventario(const TInventario *inventario) {
	for(int i =0;i <= inventario->itens->capacidade;i++){
		bar(inventario->x,inventario->y+(i*100),inventario->x+100,inventario->y+i+1*100);
	}
//	putimage(0,0,camera->imagem,COPY_PUT);
}

void mostrarItensInventario(const TInventario *inventario) {
	for(int i =0;i < inventario->itens->tamanho;i++) {
		Item item = inventario->itens->itens[i]; 
		putimage(inventario->x, inventario->y + item.altura*i, item.mascara, AND_PUT);
		putimage(inventario->x, inventario->y + item.altura*i, item.imagem, OR_PUT);
	}
}

void mostrarBotoes(const BotoesVetor *botoes) {
	for(int i = 0;i< botoes->tamanho;i++){
		Botao botao = botoes->botoes[i];
		putimage(botao.x, botao.y, botao.mascara, AND_PUT);
		putimage(botao.x, botao.y, botao.imagem, OR_PUT);
	}
	
}

void mostraTempo(int tempo){
	mciSendString("open .\\Audios\\porta1.mp3 type MPEGVideo alias Porta1", NULL, 0, 0);
	mciSendString("open .\\Audios\\porta2.mp3 type MPEGVideo alias Porta2", NULL, 0, 0);
	
	
	waveOutSetVolume(0,0xFFFFFFFF);
	
	bool chamou1;
	bool chamou2;
	bool chamou3;
	bool chamou4;
	
	int last_time = GetTickCount();
	char str[20];
	int tempo_limite = 15;
	int segundos = (last_time - tempo)/1000;
	float milisegundos = (last_time - tempo);
	int count = 0;
	int gt2 = GetTickCount();
	
	if(segundos > 15) {
		segundos = 15;
	}
	
	settextstyle(SANS_SERIF_FONT,HORIZ_DIR,4);
	sprintf(str,"%d",tempo_limite - segundos);
	int tamanho_texto = textwidth(str);
	outtextxy(1280/2 - tamanho_texto/2 ,0,str);

	settextstyle(SANS_SERIF_FONT,HORIZ_DIR,2);
	
}

void executaSom(unsigned long long int *ts1){
	unsigned long long int ts2 = GetTickCount();
	mciSendString("open .\\Audios\\Porta2.mp3 type MPEGVideo alias Porta", NULL, 0, 0);
	waveOutSetVolume(0,0xFFFFFFFF);
	printf("%d\n", ts2-*ts1);
	
	if((ts2 - *ts1) == 5000 ){
		*ts1 = GetTickCount();
		printf("chamou som : %d\n",(ts2 - *ts1));
		mciSendString("seek Porta to start", NULL, 0, 0);
		mciSendString("play Porta", NULL, 0, 0);
	}
}

void mostrarBotao(const Botao *botao) {
	putimage(botao->x, botao->y, botao->mascara, AND_PUT);
	putimage(botao->x, botao->y, botao->imagem, OR_PUT);
}



void removeItensCamera(TCamera *camera) {
	for(int i =0;i<camera->qtdItens;i++) {
		Item *item = camera->itens->itens; 
		free(item->imagem);
		free(item->mascara);
		free(item);
	}
}

bool clicou = false;
//

//
bool verificaMouseClick() {
	int x;
	int y;
	bool clicou = false;
	if(ismouseclick(WM_LBUTTONDOWN)){
		getmouseclick(WM_LBUTTONDOWN,x,y);
		clearmouseclick(WM_LBUTTONDOWN);
		clicou = true;
	}
	return clicou;
}


void pegarItem(Item *_item, TCamera *camera, TInventario *inventario){

	for(int i = 0;i<camera->itens->tamanho;i++){
		Item item = camera->itens->itens[i];
		if(item.id == _item->id){
			Item item = camera->itens->itens[i];
			remove_item_vetor(camera->itens,&item);
			append_vetor_itens(inventario->itens,&item);
		}
	}

}

void colisaoMouseBotao(BotoesVetor* botoes){
	POINT P;
  	HWND janela;
  	janela = GetForegroundWindow();

  	mciSendString("open .\\Audios\\MusicaTema.mp3 type MPEGVideo alias Tema", NULL, 0, 0);
	waveOutSetVolume(0,0xFFFFFFFF);
	mciSendString("play Tema repeat", NULL, 0, 0);

  	
  	if (GetCursorPos(&P)) // captura a posição do mouse. A variável P é carregada com coordenadas físicas de tela
        if (ScreenToClient(janela, &P)) 
  	
	for(int i = 0;i<botoes->tamanho;i++) {
		Botao botao = botoes->botoes[i];

		if (P.x < botao.x + botao.largura && P.x > botao.x && P.y < botao.y + botao.altura && P.y > botao.y) {

			bar(botao.x,botao.y + (botao.altura - 2.5),botao.x+botao.largura,botao.y+botao.altura);
			delay(50);
    		if(GetKeyState(VK_LBUTTON)&0x80){
    			if(botao.nome == "iniciar"){
    				mciSendString("stop Tema", NULL, 0, 0);
    				comecaJogo();
				} else if(botao.nome == "intro") {
					delay(200);
					Tutorial();
				} else if(botao.nome == "voltar") {
					delay(200);
					Menu();
				}  else if (botao.nome == "sair") {
					delay(200);
					mciSendString("stop Tema", NULL, 0, 0);
					Animacao();
					printf("clicou no sair\n");
				}
			}
		}
	}
}

void colisaoMouseItens(TCamera camera, TInventario *inventario) {
	
	POINT P;
  	HWND janela;
  	janela = GetForegroundWindow();
  	
  	if (GetCursorPos(&P)) // captura a posição do mouse. A variável P é carregada com coordenadas físicas de tela
        if (ScreenToClient(janela, &P)) 
  	
	for(int i = 0;i<camera.itens->tamanho;i++) {
		Item item = camera.itens->itens[i];

		if (P.x < item.x + item.largura && P.x > item.x && P.y < item.y + item.altura && P.y > item.y) {
			int largura_texto = textwidth(item.nome);
			outtextxy(item.x + ((item.largura/2) - (largura_texto/2)), item.y+item.altura,item.nome);
			delay(50);
    		if(GetKeyState(VK_LBUTTON)&0x80){
    			pegarItem(&item,&camera,inventario);
			}

		}
	}
	
}

void colisaoMouseSaidas(TCamera camera, TInventario *inventario) {
	
	POINT P;
  	HWND janela;
  	janela = GetForegroundWindow();
  	
  	if (GetCursorPos(&P))
        if (ScreenToClient(janela, &P)) 
  	
	if(camera.saida != NULL) {
		Saida *saida = camera.saida;

		if (P.x < saida->x + saida->largura && 
			P.x > saida->x && 
			P.y < saida->y + saida->altura && 
			P.y > saida->y) 
		{
			int largura_texto = textwidth(saida->nome);
			int altura_texto = textheight(saida->nome);
			outtextxy(saida->x + ((saida->largura/2) - (largura_texto/2)), saida->y + ((saida->altura/2) - (altura_texto/2)),saida->nome);
			
			delay(50);
			
    		if(GetKeyState(VK_LBUTTON)&0x80){
    			
    			printf("clicou na saida 123:%s\n", saida->nome);
    			
    			print_vetor_finais(camera.saida->finais);
    			
    			for(int i = 0;i< saida->finais->tamanho; i++){
    				compara_vetor_itens(saida->finais->finais[i],inventario->itens);
				}
    			
				delay(500);	
			}	
		}	
	}	
}

void mudarDeCamera(int *camera_atual,int *tecla) {
	if(GetKeyState(VK_LEFT)&0X80) {
		*camera_atual -= 1;
		if(*camera_atual < 0) {
        	*camera_atual = 3;
		}
		delay(200);
	}
	if(GetKeyState(VK_RIGHT)&0X80) {
		*camera_atual += 1;
		if(*camera_atual > 3) {
        	*camera_atual = 0;
		}
		delay(200);
	} 
}

unsigned long long int gt1 = GetTickCount();

int Animacao()  { 
  int pg, Porta, i, um = 1, xt = 0;
  //delay(4000);
  void **t;                                         
  t = (void **)malloc(sizeof(void *) * QtdCenas);  
  
  Porta = imagesize(0, 0, 1280,720);

  Final *final0 = criar_final(0,"fuga de carro","Lorem ipsum dolor sit amet, consectetur adipiscing elit. Pellentesque id dignissim quam. Proin sit amet pulvinar nulla, et sagittis magna. Curabitur congue consectetur sollicitudin. Suspendisse aliquam lorem a est tincidunt semper. Vestibulum sed hendrerit elit. Praesent lorem ex, lacinia vel");
  Final *final1 = criar_final(1,"matar monstro","Lorem ipsum dolor sit amet, consectetur adipiscing elit. Pellentesque id dignissim quam. Proin sit amet pulvinar nulla, et sagittis magna. Curabitur congue consectetur sollicitudin. Suspendisse aliquam lorem a est tincidunt semper. Vestibulum sed hendrerit elit. Praesent lorem ex, lacinia vel");
  Final *final2 = criar_final(2,"quebrar a janela","Lorem ipsum dolor sit amet, consectetur adipiscing elit. Pellentesque id dignissim quam. Proin sit amet pulvinar nulla, et sagittis magna. Curabitur congue consectetur sollicitudin. Suspendisse aliquam lorem a est tincidunt semper. Vestibulum sed hendrerit elit. Praesent lorem ex, lacinia vel");
  Final *final3 = criar_final(3,"botar fogo na casa","Lorem ipsum dolor sit amet, consectetur adipiscing elit. Pellentesque id dignissim quam. Proin sit amet pulvinar nulla, et sagittis magna. Curabitur congue consectetur sollicitudin. Suspendisse aliquam lorem a est tincidunt semper. Vestibulum sed hendrerit elit. Praesent lorem ex, lacinia vel");
	

  mciSendString("stop Insetos", NULL, 0, 0);

  for(i = 0; i < QtdCenas; i++)  
    t[i] = malloc(Porta);
    
	i = 0;
    readimagefile(".\\Animacao\\K_0001.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[0]);
    readimagefile(".\\Animacao\\K_0002.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[1]);
    readimagefile(".\\Animacao\\K_0003.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[2]);
    readimagefile(".\\Animacao\\K_0004.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[3]);
    readimagefile(".\\Animacao\\K_0005.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[4]);
    readimagefile(".\\Animacao\\K_0006.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[5]);
    readimagefile(".\\Animacao\\K_0007.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[6]);
    readimagefile(".\\Animacao\\K_0008.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[7]);
    readimagefile(".\\Animacao\\K_0009.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[8]);
    readimagefile(".\\Animacao\\K_0010.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[9]);
    readimagefile(".\\Animacao\\K_0011.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[10]);
    readimagefile(".\\Animacao\\K_0012.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[11]);
    readimagefile(".\\Animacao\\K_0013.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[12]);
    readimagefile(".\\Animacao\\K_0014.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[13]);
    readimagefile(".\\Animacao\\K_0015.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[14]);
    readimagefile(".\\Animacao\\K_0016.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[15]);
    readimagefile(".\\Animacao\\K_0017.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[16]);
    readimagefile(".\\Animacao\\K_0018.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[17]);
    readimagefile(".\\Animacao\\K_0019.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[18]);
    readimagefile(".\\Animacao\\K_0020.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[19]);
    readimagefile(".\\Animacao\\K_0021.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[20]);
    readimagefile(".\\Animacao\\K_0022.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[21]);
    readimagefile(".\\Animacao\\K_0023.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[22]);
    readimagefile(".\\Animacao\\K_0024.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[23]);
    readimagefile(".\\Animacao\\K_0025.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[24]);
    readimagefile(".\\Animacao\\K_0026.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[25]);
    readimagefile(".\\Animacao\\K_0027.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[26]);
    readimagefile(".\\Animacao\\K_0028.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[27]);
    readimagefile(".\\Animacao\\K_0029.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[28]);
    readimagefile(".\\Animacao\\K_0030.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[29]);
    readimagefile(".\\Animacao\\K_0031.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[30]);
    readimagefile(".\\Animacao\\K_0032.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[31]);
    readimagefile(".\\Animacao\\K_0033.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[32]);
    readimagefile(".\\Animacao\\K_0034.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[33]);
    readimagefile(".\\Animacao\\K_0035.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[34]);
    readimagefile(".\\Animacao\\K_0036.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[35]);
    readimagefile(".\\Animacao\\K_0037.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[36]);
    readimagefile(".\\Animacao\\K_0038.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[37]);
    readimagefile(".\\Animacao\\K_0039.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[38]);
    readimagefile(".\\Animacao\\K_0040.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[39]);
    readimagefile(".\\Animacao\\K_0041.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[40]);
    readimagefile(".\\Animacao\\K_0042.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[41]);
    readimagefile(".\\Animacao\\K_0043.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[42]);
    readimagefile(".\\Animacao\\K_0044.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[43]);
    readimagefile(".\\Animacao\\K_0045.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[44]);
    readimagefile(".\\Animacao\\K_0046.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[45]);
    readimagefile(".\\Animacao\\K_0047.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[46]);
    readimagefile(".\\Animacao\\K_0048.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[47]);
    readimagefile(".\\Animacao\\K_0049.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[48]);
    readimagefile(".\\Animacao\\K_0050.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[49]);
    readimagefile(".\\Animacao\\K_0051.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[50]);
    readimagefile(".\\Animacao\\K_0052.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[51]);
    readimagefile(".\\Animacao\\K_0053.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[52]);
    readimagefile(".\\Animacao\\K_0054.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[53]);
    readimagefile(".\\Animacao\\K_0055.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[54]);
    readimagefile(".\\Animacao\\K_0056.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[55]);
    readimagefile(".\\Animacao\\K_0057.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[56]);
    readimagefile(".\\Animacao\\K_0058.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[57]);
    readimagefile(".\\Animacao\\K_0059.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[58]);
    readimagefile(".\\Animacao\\K_0060.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[59]);
    readimagefile(".\\Animacao\\K_0061.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[60]);
    readimagefile(".\\Animacao\\K_0062.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[61]);
    readimagefile(".\\Animacao\\K_0063.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[62]);
    readimagefile(".\\Animacao\\K_0064.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[63]);
    readimagefile(".\\Animacao\\K_0065.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[64]);
    readimagefile(".\\Animacao\\K_0066.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[65]);
    readimagefile(".\\Animacao\\K_0067.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[66]);
    readimagefile(".\\Animacao\\K_0068.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[67]);
    readimagefile(".\\Animacao\\K_0069.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[68]);
    readimagefile(".\\Animacao\\K_0070.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[69]);
    readimagefile(".\\Animacao\\K_0071.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[70]);
    readimagefile(".\\Animacao\\K_0072.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[71]);
    readimagefile(".\\Animacao\\K_0073.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[72]);
    readimagefile(".\\Animacao\\K_0074.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[73]);
    readimagefile(".\\Animacao\\K_0075.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[74]);
    readimagefile(".\\Animacao\\K_0076.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[75]);
    readimagefile(".\\Animacao\\K_0077.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[76]);
    readimagefile(".\\Animacao\\K_0078.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[77]);
    readimagefile(".\\Animacao\\K_0079.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[78]);
    readimagefile(".\\Animacao\\K_0080.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[79]);
    readimagefile(".\\Animacao\\K_0081.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[80]); 
    i = 0;
  while(true) {
    if (pg == 1) pg = 2; else pg=1;
    setactivepage(pg);
    setbkcolor(RGB(0, 0, 0));
    cleardevice();
    
    for(xt = 0; xt < 7; xt++) 
    {
      putimage(0, 0, t[i], COPY_PUT);
    }
    i++;
    if(i == 80){
       Conclusao(Final(*final0));	
	}
    //Conclusao(Final(*final0));
    
  }
  
  return 0; 
}


int Conclusao(Final final) {
	int count = 0;
	
	mciSendString("stop Insetos", NULL, 0, 0);
	
	settextstyle(SANS_SERIF_FONT,HORIZ_DIR,2);
	
	int LarTela;
	LarTela = 1100;
	unsigned long long int gt2;
	
	char *texto = final.historia;
	

	
	delay(2000);
	
	while(true) {
 		gt2 = GetTickCount();
 		
		if(gt2 - gt1 > 1000/60) {	
			if(pg == 1) pg = 2; else pg = 1;
 			setvisualpage(pg);
 			cleardevice();
 			setfillstyle(0,RGB(0,0,0));
 			bar(0,0,1280,720);
 			if(count == 0) {
 				animacao_texto(texto,LarTela,583,50,50);
				count++;	
			}
			setactivepage(pg);
		}
	}
	return 0;
}

int comecaJogo(){
	unsigned long long int tempo = GetTickCount();
	unsigned long long int gt2;
	unsigned long long int ts1 = GetTickCount();
    int tecla = 0;
	int camera_atual = 0;
	int qtdCam = 0;
	int LarTela,AltTela;
	
	LarTela = 1280;
	AltTela = 720;
	
	setlocale(LC_ALL,"Portuguese");
	settextstyle(SANS_SERIF_FONT,HORIZ_DIR,2);
	
	
	TMouse *mouse = mousePos();


	
	mciSendString("open .\\Audios\\BugsSound.mp3 type MPEGVideo alias Insetos", NULL, 0, 0);
	waveOutSetVolume(0,0x88888888);
	mciSendString("play Insetos repeat", NULL, 0, 0);
	
	
	void* item1_img = load_image("dinamite.bmp",100,100,200,200);
	void* item1_mask = load_image("dinamite_pb.bmp",100,100,200,200);
	
	Item *dinamite1 = criar_item(0,"dinamite1",item1_img,item1_mask,200,200,100,100);
    Item *dinamite2 = criar_item(1,"dinamite2",item1_img,item1_mask,300,200,100,100);
    Item *dinamite3 = criar_item(2,"dinamite3",item1_img,item1_mask,400,200,100,100);
    Item *dinamite4 = criar_item(3,"dinamite4",item1_img,item1_mask,500,200,100,100);
    Item *dinamite5 = criar_item(4,"dinamite5",item1_img,item1_mask,600,200,100,100);
    Item *dinamite6 = criar_item(5,"dinamite6",item1_img,item1_mask,700,200,100,100);
    //variaveis do jogo
    
	Final *final0 = criar_final(0,"fuga de carro","Lorem ipsum dolor sit amet, consectetur adipiscing elit. Pellentesque id dignissim quam. Proin sit amet pulvinar nulla, et sagittis magna. Curabitur congue consectetur sollicitudin. Suspendisse aliquam lorem a est tincidunt semper. Vestibulum sed hendrerit elit. Praesent lorem ex, lacinia vel");
	Final *final1 = criar_final(1,"matar monstro","Lorem ipsum dolor sit amet, consectetur adipiscing elit. Pellentesque id dignissim quam. Proin sit amet pulvinar nulla, et sagittis magna. Curabitur congue consectetur sollicitudin. Suspendisse aliquam lorem a est tincidunt semper. Vestibulum sed hendrerit elit. Praesent lorem ex, lacinia vel");
	Final *final2 = criar_final(2,"quebrar a janela","Lorem ipsum dolor sit amet, consectetur adipiscing elit. Pellentesque id dignissim quam. Proin sit amet pulvinar nulla, et sagittis magna. Curabitur congue consectetur sollicitudin. Suspendisse aliquam lorem a est tincidunt semper. Vestibulum sed hendrerit elit. Praesent lorem ex, lacinia vel");
	Final *final3 = criar_final(3,"botar fogo na casa","Lorem ipsum dolor sit amet, consectetur adipiscing elit. Pellentesque id dignissim quam. Proin sit amet pulvinar nulla, et sagittis magna. Curabitur congue consectetur sollicitudin. Suspendisse aliquam lorem a est tincidunt semper. Vestibulum sed hendrerit elit. Praesent lorem ex, lacinia vel");
	
	final0->itens = criar_vetor_itens(2);
	
	append_vetor_itens(final0->itens,dinamite2);
	append_vetor_itens(final0->itens,dinamite5);
	
	FinaisVetor *finais0 = criar_vetor_finais(4);
	
	append_vetor_finais(finais0,final0);
	append_vetor_finais(finais0,final1);
	append_vetor_finais(finais0,final2);
	append_vetor_finais(finais0,final3);
	
	print_vetor_finais(finais0);
	print_vetor_itens(final0->itens);
	
	Saida *saida0 = criarSaida(0,"porta",1300,300,200,500);
	Saida *saida1 = criarSaida(0,"janela",50,300,750,300);
	
	saida0->finais = criar_vetor_finais(6);
	saida1->finais = criar_vetor_finais(6);
	
	append_vetor_finais(saida1->finais,final0);
	append_vetor_finais(saida1->finais,final1);
	append_vetor_finais(saida1->finais,final2);
	append_vetor_finais(saida1->finais,final3);
	
	print_vetor_finais(saida0->finais);
	
	void *img_cam0 = load_image("quarto0.bmp",LarTela,AltTela,0,0);
	void *img_cam1 = load_image("quarto1.bmp",LarTela,AltTela,0,0);
	void *img_cam2 = load_image("quarto2.bmp",LarTela,AltTela,0,0);
	void *img_cam3 = load_image("quarto3.bmp",LarTela,AltTela,0,0);
	
	TCamera *camera0 = criarCamera(0,img_cam0);
	camera0->saida = saida0;
	
	TCamera *camera1 = criarCamera(1,img_cam1);
	
	TCamera *camera2 = criarCamera(2,img_cam2);
	camera2->saida = saida1;
	
	TCamera *camera3 = criarCamera(3,img_cam3);
 	
 	TInventario *inventario = criar_inventario(LarTela-100,0);
 	inventario->itens = criar_vetor_itens(2);
 	
 	TCamera *cameras = (TCamera*)malloc(sizeof(TCamera)*4);
 	
 	cameras[0] = *camera0;
 	cameras[1] = *camera1;
 	cameras[2] = *camera2;
 	cameras[3] = *camera3;
 	
 	cameras[0].itens = criar_vetor_itens(20);
 	cameras[1].itens = criar_vetor_itens(20);
 	cameras[2].itens = criar_vetor_itens(20);
 	cameras[3].itens = criar_vetor_itens(20);
 	
 	append_vetor_itens(cameras[0].itens,dinamite1);
 	append_vetor_itens(cameras[0].itens,dinamite2);
 	append_vetor_itens(cameras[1].itens,dinamite3);
 	append_vetor_itens(cameras[3].itens,dinamite4);
	append_vetor_itens(cameras[1].itens,dinamite5);
	append_vetor_itens(cameras[3].itens,dinamite6);
	
	print_vetor_finais(saida0->finais);
	
 	while(true) {
 		gt2 = GetTickCount();
 		
 		if(gt2 - gt1 > 1000/60) {
 			if(pg == 1) pg = 2; else pg = 1;
 			setvisualpage(pg);
 			cleardevice();
 			
 			mudarDeCamera(&camera_atual,&tecla);

 			mostrarCamera(&cameras[camera_atual]);
			mostrarItensCamera(&cameras[camera_atual]);
			
			mostrarInventario(inventario);
			mostrarItensInventario(inventario);
			mostrarSaidasCamera(&cameras[camera_atual]);
			mostraTempo(tempo);
			executaSom(&ts1);
			colisaoMouseItens(cameras[camera_atual],inventario);
			colisaoMouseSaidas(cameras[camera_atual],inventario);
			
			if(((gt2 - tempo) /1000) > 15) {
				Conclusao(Final(*final0));
			}
			
 			setactivepage(pg);
		}
		
	}
	
	deleteImage(img_cam0);
	deleteImage(img_cam1);
	deleteImage(img_cam2);
	deleteImage(img_cam3);

	
	deleteImage(item1_img);
	deleteImage(item1_mask);
	
	apaga_vetor_itens(&final0->itens);
	
	apaga_vetor_finais(&saida0->finais);
	apaga_vetor_finais(&saida1->finais);
	free(saida0);
	free(saida1);
	apaga_vetor_itens(&cameras[0].itens);
 	free(cameras);
	return 0;
}

int Tutorial(){

	int LarTela,AltTela;
	
	LarTela = 1280;
	AltTela = 720;
	
	void *botao1_img = load_image(".\\Hud\\Voltar.bmp",208,84,0,0);
	void *botao1_mask = load_image(".\\Hud\\VoltarWB.bmp",208,84,0,0);
	
	Botao *botao_voltar = criar_botao("voltar",botao1_img,botao1_mask,50,600,208,84);
	
	BotoesVetor *botoes = criar_vetor_botoes(1);
	
	append_vetor_botoes(botoes,botao_voltar);
	
	void *img_menu = load_image("Tutorial.bmp",LarTela,AltTela,0,0);
	
//	Botao *botao_voltar = criar_botao("voltar",0,0,30,AltTela - 50,100,50);
	
	unsigned long long int gt2;
	
	while(true){
		
		gt2 = GetTickCount();
		if(gt2 - gt1 > 1000/60) {
			if(pg == 1) pg = 2; else pg = 1;

	 		setvisualpage(pg);	
			cleardevice();
	
			putimage(0,0,img_menu,COPY_PUT);
			mostrarBotoes(botoes);
			colisaoMouseBotao(botoes);

			setactivepage(pg);		
		}
	}
	
	deleteImage(img_menu);
	return 0;
}

int Menu(){

	int pg;
	int LarTela,AltTela;
    
    LarTela = 1280;
	AltTela = 720;
    
    void *botao1_img = load_image(".\\Hud\\Iniciar1.bmp",206,74,0,0);
	void *botao1_mask = load_image(".\\Hud\\Iniciar1WB.bmp",206,74,0,0);
    
    void *botao2_img = load_image(".\\Hud\\Iniciar2.bmp",100,50,0,0);
	void *botao2_mask = load_image(".\\Hud\\Iniciar2WB.bmp",100,50,0,0);
	
	void *botao3_img = load_image(".\\Hud\\Instrução1.bmp",330,80,0,0);
	void *botao3_mask = load_image(".\\Hud\\Instrução1WB.bmp",330,80,0,0);
	
	void *botao4_img = load_image(".\\Hud\\Sair1.bmp",142,74,0,0);
	void *botao4_mask = load_image(".\\Hud\\Sair1WB.bmp",142,74,0,0);
    
    Botao *botao_iniciar = criar_botao("iniciar",botao1_img,botao1_mask,LarTela/2 - 206/2,AltTela/2 + 75,206,74);

    Botao *botao_intro = criar_botao("intro",botao3_img,botao3_mask,LarTela/2 - 330/2,AltTela/2 +150,330,80); 

    Botao *botao_sair = criar_botao("sair",botao4_img,botao4_mask,LarTela/2 - 142/2,AltTela/2+240,142,74); 
	
	BotoesVetor *botoes = criar_vetor_botoes(3);
	
	append_vetor_botoes(botoes,botao_iniciar);
	append_vetor_botoes(botoes,botao_intro);
	append_vetor_botoes(botoes,botao_sair);

    void *img_menu = load_image("HorrorHut.bmp",LarTela,AltTela,0,0);

	unsigned long long int gt2;
	
	POINT P;
	HWND janela;
	janela = GetForegroundWindow();
	
	while(true){
		gt2 = GetTickCount();
		if(gt2 - gt1 > 1000/60) {
			if(pg == 1) pg = 2; else pg = 1;
	 		setvisualpage(pg);
	 		cleardevice();
			putimage(0,0,img_menu,COPY_PUT);
			mostrarBotoes(botoes);
			colisaoMouseBotao(botoes);
			setactivepage(pg);
		}
	}
	
	deleteImage(botao1_img);
	deleteImage(botao2_img);
	deleteImage(botao3_img);
	deleteImage(botao4_img);
	
	deleteImage(botao1_mask);
	deleteImage(botao2_mask);
	deleteImage(botao3_mask);
	deleteImage(botao4_mask);
	
	apaga_vetor_botoes(&botoes);
	return 0;
}

//TItem *chave = criarItem(1,"chave","chave.bmp","chave_pb.bmp",200,200,100,100);
//TItem *gasolina = criarItem(2,"gasolina","gasolina.bmp","gasolina_pb.bmp",400,600,50,100);
//TItem *rifle = criarItem(3,"rifle","rifle.bmp","rifle_pb.bmp",400,600,50,100);
//TItem *bala = criarItem(4,"bala","bala.bmp","bala_pb.bmp",400,600,50,100);
//TItem *celular = criarItem(5,"celular","celular.bmp","celular_pb.bmp",400,600,50,100);
//TItem *machado = criarItem(6,"machado","machado.bmp","machado_pg.bmp",400,600,50,100);
//TItem *fosforo = criarItem(7,"fosforo","fosforo.bmp","fosforo_pb.bmp",400,600,50,100);
//TItem *armadilha = criarItem(8,"armadilha","armadilha.bmp","armadilha_pb.bmp",400,600,50,100);

int main() {
	
	int LarTela = 1280;
	int AltTela = 720;
	
	setlocale(LC_ALL,"Portuguese");
	initwindow(LarTela, AltTela,"meu jogo");
	Menu();
	TMouse *mouse = mousePos();
 	closegraph();
	return 0;
}
