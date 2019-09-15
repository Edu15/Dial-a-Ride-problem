# altere a variável PRINCIPAL para o nome do seu programa principal (omita o .cpp)
PRINCIPAL = darp
# inclua aqui todas as dependências do seu programa principal
OBJS = cost.o essentials.o localSearch.o moves.o util.o commandShell.o
INCLUDES = darp.h

all: $(PRINCIPAL)
compile: $(PRINCIPAL)

$(PRINCIPAL): $(PRINCIPAL).o $(OBJS)
	gcc -g $(PRINCIPAL).o $(OBJS) -o $@ -lm

$(PRINCIPAL).o: $(PRINCIPAL).c $(INCLUDES)
	gcc -g -c $(PRINCIPAL).c -o $(PRINCIPAL).o -lm

# compila todos os arquivos .cpp que tem includes
%.o: %.c %.h
	gcc -g -c $*.c -o $@ -lm

clean:
	rm -rf *.o
	rm -rf $(PRINCIPAL)

run:
	./$(PRINCIPAL) ./instancia/pr01

runv:
	valgrind --track-origins=yes ./$(PRINCIPAL) ./instancia/pr01
	
run2:
	./$(PRINCIPAL) ./instancia/pr02

runv2:
	valgrind --track-origins=yes ./$(PRINCIPAL) ./instancia/pr01

