#include "seq.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

/*Ciagi sa przechowywane w drzewie z trzema synami.
*
* Każdy węzel ma pole abstract_class_name w ktorym znajduje się jego
* nazwa klasy abstrakcji.
*
* abstract_class zawiera numer klasy abstrakcji
* w której znajduje się dany węzeł. Ciagi bez klasy abstrakcji maja domyslnie ta
* wartosć zapisana na -1.
*
* abstract_classes_amount jest wskaźnikiem do zmiennej która mówi ile jest
* obecnie klas abstrakcji w magazynie i jest używana do wybierania numeru
* abstract_class nowym klasom abstrakcji.
*/
typedef struct seq {
    struct seq * next_zero;
    struct seq * next_one;
    struct seq * next_two;
    char * abstract_class_name;
    int abstract_class;
    int * abstract_classes_amount;
} seq_t;

/*Inicjuje nowa strukture do przechowywania ciagow.
* Tworzy korzen drzewa w ktorym przechowywane sa ciagi.
*/
seq_t * seq_new(void) {
    seq_t * return_seq = (seq_t *) malloc(sizeof(seq_t));
    int * abstract_classes = (int *) malloc(sizeof(int));

    if (!return_seq || !abstract_classes) {
        errno = ENOMEM;
        if (return_seq) {
            free(return_seq);
            return_seq = NULL;
        }
        if (abstract_classes) {
            free(abstract_classes);
            abstract_classes = NULL;
        }
        return NULL;
    }

    *abstract_classes = 0;
    return_seq->next_one = NULL;
    return_seq->next_two = NULL;
    return_seq->next_zero = NULL;
    return_seq->abstract_class = -1;
    return_seq->abstract_classes_amount = abstract_classes;
    return_seq->abstract_class_name = NULL;
    
    return return_seq;
}

/*Sprawdza czy w wyrazie nie ma nielegalnych symboli.
* W takim przypadku zwraca -1 i ustawie errno na EINVAL.
* Gdy wyraz jest poprawny zwraca 0.
*/
int check_str_for_inval(char const * s) {
    if (!s) {
        errno = EINVAL;
        return -1;
    }
    
    int length = (int) strlen(s);
    if (!length) {
        errno = EINVAL;
        return -1;
    }

    for (int i = 0; i < length; i++) {
        char current_num = s[i];
        if (current_num != '0' && current_num != '1' && current_num != '2') {
            errno = EINVAL;
            return -1;
        }
    }
    return 0;
}

/*Sprawdza czy z danego węzla magazynu wyrazow p wychodzi węzel dla danego
* symbolu.
*/
bool check_seq_for_next(seq_t * p, char val) {
    if (val == '0' && p->next_zero) return true;
    if (val == '1' && p->next_one) return true;
    if (val == '2' && p->next_two) return true;
    return false;
}

/*Wykonuje probę dodania węzla za węzel magazynu wyrazow p.
*
* Jesli taki węzel już tam istnieje zwraca zero i nie robi nic
*
* W przypadku błedu mallocowania pamieci zwraca -1 i przypisuje
* errno wartosc ENOMEM.
*/
int new_seq_node(seq_t * p, char new_val) {
    if (new_val != '0' && new_val != '1' && new_val != '2') {
        errno = EINVAL;
        return -1;
    }

    if (check_seq_for_next(p, new_val)) return 0;

    seq_t * temp = (seq_t *) malloc(sizeof(seq_t));
    
    if (temp == NULL) {
        errno = ENOMEM;
        return -1;
    }

    temp->abstract_class_name = NULL;
    temp->abstract_class = -1;
    temp->abstract_classes_amount = p->abstract_classes_amount;
    temp->next_one = NULL;
    temp->next_two = NULL;
    temp->next_zero = NULL;

    if (new_val == '0') p->next_zero = temp;
    if (new_val == '1') p->next_one = temp;
    if (new_val == '2') p->next_two = temp;

    return 1;
}

/*Zwracanie następnego węzła obrazjącego 0 1 2*/
seq_t * next_seq(seq_t * p, char next_char) {
    //printf("\nw next\n");
    if (!p) return NULL;
    switch (next_char) {
        case '0':
            if (p->next_zero) return p->next_zero;
            break;
        
        case '1':
            if (p->next_one) return p->next_one;
            break;

        case '2':
            if (p->next_two) return p->next_two;
            break;
        
        default:
            return NULL;
    }
    return NULL;
}

/*Usuwanie ciągu kończącego się na węźle p i wszystkich ciągów których jest
* prefiksem rekurencyjnie.
*/
void seq_remove_recur(seq_t * p) {
    if (p != NULL) {
        if (p->next_zero != NULL) {
            seq_remove_recur(p->next_zero);
            p->next_zero = NULL;
        }
        if (p->next_one != NULL) {
            seq_remove_recur(p->next_one);
            p->next_one = NULL;
        }
        if (p->next_two) {
            seq_remove_recur(p->next_two);
            p->next_two = NULL;
        }

        if (p->abstract_class_name) free(p->abstract_class_name);
        p->abstract_classes_amount = NULL;
        free(p);
    }
}

/*Dodaje do struktury ciąg s i wszystkie jego prefiksy.
* W przypadku błędu allokacji usuwa wszystkie dodane już węzły.
*/
int seq_add(seq_t * p, char const * s) {
    if (!p) {
        errno = EINVAL;
        return -1;
    }
    if (!s) {
        errno = EINVAL;
        return -1;
    }

    int length = (int) strlen(s);
    int added_seqs = 0;

    if (check_str_for_inval(s) == -1) return -1;
    
    seq_t * first_added_seq = NULL;
    seq_t * current_seq_node = p;
    seq_t * second_to_last = p;
    int first_index = 0;

    for (int i = 0; i < length; i++) {
        int current_adding_result = new_seq_node(current_seq_node, s[i]);
        if (first_added_seq == NULL && current_adding_result == 1) {
            second_to_last = current_seq_node;
            first_index = i;
            first_added_seq = next_seq(current_seq_node, s[i]);
        }

        if (current_adding_result == -1) {
            if (first_added_seq) {
                seq_remove_recur(first_added_seq);
                switch (s[first_index]) {
                    case '0':
                        second_to_last->next_zero = NULL;
                        break;
        
                    case '1':
                        second_to_last->next_one = NULL;
                        break;

                    case '2':
                        second_to_last->next_two = NULL;
                        break;
            }
            }
            errno = ENOMEM;
            return -1;
        }
        else {
            added_seqs += current_adding_result;
            current_seq_node = next_seq(current_seq_node, s[i]);
        }
        
    }
    
    return added_seqs > 0;
    
}

/*Usuwa ze struktury ciąg s i wszystkie ciągi których jest prefiksem.*/
int seq_remove(seq_t * p, char const * s) {
    if (!p) {
        errno = EINVAL;
        return -1;
    }
    seq_t * current_seq = p;
    
    if (check_str_for_inval(s) == -1) return -1;
    int length = (int) strlen(s);
    seq_t * second_to_last = p;



    for (int i = 0; i < length; i++) {
        if (i == length - 1) {
            second_to_last = current_seq;
        }

        if (!check_seq_for_next(current_seq, s[i])) return 0;
        current_seq = next_seq(current_seq, s[i]); 
    }
    
    seq_remove_recur(current_seq);
    switch (s[length - 1]) {
        case '0':
            second_to_last->next_zero = NULL;
            break;
        
        case '1':
            second_to_last->next_one = NULL;
            break;

        case '2':
            second_to_last->next_two = NULL;
            break;
    }

    return 1;
}

/*Usuwa zbiór ciągów i zwalnia całą używaną przez niego pamięć*/
void seq_delete(seq_t * p) {
    if (p) {
        seq_remove(p, "0");
        seq_remove(p, "1");
        seq_remove(p, "2");
        free(p->abstract_classes_amount);
        if (p->abstract_class_name)
            free(p->abstract_class_name);
        free(p);
        p = NULL;
    }
}

/*Sprawdza czy podany ciag nalezy do ciagu zbiorow.*/
int seq_valid(seq_t * p, char const * s) {
    if (check_str_for_inval(s) == -1) return -1;
    if (!p) {
        errno = EINVAL;
        return -1;
    }
    
    seq_t * current_seq = p;
    int length = (int) strlen(s);

     for (int i = 0; i < length; i++) {
        if (!check_seq_for_next(current_seq, s[i])) return 0;
        current_seq = next_seq(current_seq, s[i]);  
    } 
    return 1;
}

/* Podmienia imię ciągu przedstawionego w węźle lub ustawia całkowicie nowe.*/
int replace_or_set_seq_name(seq_t * p, char const * n) {
    int n_length = strlen(n) + 1;
    if (p) {
        if (p->abstract_class_name) {
            free(p->abstract_class_name);
            p->abstract_class_name = NULL;
        }    
        
        p->abstract_class_name = (char *) malloc(sizeof(char) * n_length);
        if (!p->abstract_class_name) {
            errno = ENOMEM;
            return -1;
        }

        strcpy(p->abstract_class_name, n);
            
        return 1;
        
    }
    return 0;
}

/*Przechodzi po całej strukturze ustawiając imię n wszystkim węzłom w klasie
* abstrakcji current_abs_class.
*/
int recur_seq_name(seq_t * p, char const * n, int current_abs_class) {
    int next_zero;
    int next_one;
    int next_two;

    if (!p) return 0;
    else {
        next_zero = recur_seq_name(p->next_zero, n, current_abs_class);
        next_one = recur_seq_name(p->next_one, n, current_abs_class);
        next_two = recur_seq_name(p->next_two, n, current_abs_class);
    
        if (next_zero == -1 || next_one == -1 || next_two == -1) return -1;

        if (p->abstract_class == current_abs_class)
            return (replace_or_set_seq_name(p, n) == 1);

        else return 1;
    }
}

/*Ustawia imię n ciągowi s i wszystkim innym ciągom
* w tej samej klasie abstrakcji.
*/
int seq_set_name(seq_t * p, char const * s, char const * n) {
    if (!p) {
        errno = EINVAL;
        return -1;
    }

    if (check_str_for_inval(s) == -1) return -1;

    if (!n) {    
        errno = EINVAL;
        return -1;
    }
    
    seq_t * current_seq = p;
    int length = (int) strlen(s);
    int n_length = (int) strlen(n) + 1;
    
    if (!(n_length - 1)) {    
        errno = EINVAL;
        return -1;
    }

    for (int i = 0; i < length; i++) {
        if (!check_seq_for_next(current_seq, s[i])) return 0;
        current_seq = next_seq(current_seq, s[i]);  
    } 

    int current_abs_class = current_seq->abstract_class;
    if (current_abs_class == -1) {
        current_seq->abstract_class_name =
            (char *) malloc(sizeof(char) * n_length);
        
        if (!current_seq->abstract_class_name) {
            errno = ENOMEM;
            return -1;
        } 
        else {
            for (int i = 0; i < n_length; i++) {
                (current_seq->abstract_class_name)[i] = n[i];
            }   
        }
    
        current_abs_class = *current_seq->abstract_classes_amount;
        *current_seq->abstract_classes_amount += 1;
        current_seq->abstract_class = *current_seq->abstract_classes_amount;
        return 1;
    }
    else {
        char * current_name =  current_seq->abstract_class_name;
        if (current_name && !strcmp(n, current_name)) return 0;
        return recur_seq_name(p, n, current_abs_class);
    }
}

/*Zwraca imię przypisane ciągowi s ze struktury p.*/
char const * seq_get_name(seq_t * p, char const * s) {
    if (!p) {
        errno = EINVAL;
        return NULL;
    }

    if (check_str_for_inval(s) == -1) return NULL;
    seq_t * current_seq = p;
    int length = (int) strlen(s);

    for (int i = 0; i < length; i++) {
        if (!check_seq_for_next(current_seq, s[i])) {
            errno = 0;
            return NULL;
        }
        current_seq = next_seq(current_seq, s[i]);  
    }

    if (current_seq && !current_seq->abstract_class_name) {
        errno = 0;
        return NULL;
    } else {
        return current_seq->abstract_class_name;
    }
}

/*Kopiuje string z imieniem n i zwrace go.*/
char * singular_name(char const * n) {
    int n_len = (int) strlen(n);

    char * new_name = (char *)malloc(sizeof(int) * (n_len + 1));
    if (new_name == NULL) {
        errno = ENOMEM;
        return NULL;
    }

    if (n) {
        strcpy(new_name, n);
    }

    return new_name;
}

/*Łączy dwa stringi z imionami ciągów w jeden który tworzy a następnie zwraca.*/
char * merge_names(char const * n1, char const * n2) {
    int new_name_length = 1;
    int n1_len = 0;
    int n2_len = 0;

    if (n1) {
        n1_len = (int)strlen(n1);
        new_name_length += n1_len;
    }
    if (n2) {
        n2_len = (int)strlen(n2);
        new_name_length += n2_len;
    }

    char * new_name = (char *)malloc(sizeof(int) * new_name_length);
    if (new_name == NULL) {
        errno = ENOMEM;
        return NULL;
    }

    if (n1) {
        strcpy(new_name, n1);
    }
    if (n2 && n1) {
        strcat(new_name, n2);
    }
    else if (n2 && !n1) {
        strcpy(new_name, n2);
    }
    return new_name;
}

/*Zmienia klasę abstrakcji i jej nazwę w węźle*/
int node_change(
    seq_t * p, char const * n1, char const * n2, int abs_class_val
    ) {
    if (!p) return 0;
    if (p->abstract_class_name) {
        free(p->abstract_class_name);
        p->abstract_class_name = NULL;
    }

    p->abstract_class = abs_class_val;

    if (!n1 && !n2) return 1;

    if ((n1 && !n2) || !strcmp(n1, n2)) {
        p->abstract_class_name = singular_name(n1);
        if (p->abstract_class_name == NULL) return -1;
    }
    else if ((!n1 && n2) || !strcmp(n1, n2)) {
        p->abstract_class_name = singular_name(n2);
        if (p->abstract_class_name == NULL) return -1;

    }
    else {
        p->abstract_class_name = merge_names(n1, n2);
        if (p->abstract_class_name == NULL) return -1;
    }
    return 1;
}

int recur_equiv(
    seq_t * p, int abs_class_1, int abs_class_2, int abs_class_n,
    char const * name_1, char const * name_2 
    ) {
        if (!p) return 0;

        int p_abs = p->abstract_class;

        int result_zero = recur_equiv(
            p->next_zero, abs_class_1, abs_class_2, abs_class_n, name_1, name_2
        );

        int result_one = recur_equiv(
            p->next_one, abs_class_1, abs_class_2, abs_class_n, name_1, name_2
        );

        int result_two = recur_equiv(
            p->next_two, abs_class_1, abs_class_2, abs_class_n, name_1, name_2
        );

        if (result_one == -1 || result_two == -1 || result_zero == -1)
            return -1;
        
        if (p_abs != -1 && (p_abs == abs_class_1 || p_abs == abs_class_2)) {
            return node_change(p, name_1, name_2, abs_class_n);
        } else {
            return 0;
        }
        
}

/*Ustawia klasę abstrakcji dwóch ciągów na taką samą w strukturze p.
* Łączy nazwy tych ciągów w jedną.
*/
int seq_equiv(seq_t * p, char const * s1, char const * s2) {
    if (!p) {
        errno = EINVAL;
        return -1;
    }

    if (!s1 || !s2) {
        errno = EINVAL;
        return -1;
    }

    if (check_str_for_inval(s1) == -1) return -1;
    if (check_str_for_inval(s2) == -1) return -1;
    if (s1 == s2) return 0;

    int length_1 = (int)strlen(s1);
    int length_2 = (int)strlen(s2);

    seq_t * current_seq_1 = p;
    for (int i = 0; i < length_1; i++) {
        if (!check_seq_for_next(current_seq_1, s1[i])) return 0;
        current_seq_1 = next_seq(current_seq_1, s1[i]);  
    }

    seq_t * current_seq_2 = p;
    for (int i = 0; i < length_2; i++) {
        if (!check_seq_for_next(current_seq_2, s2[i])) return 0;
        current_seq_2 = next_seq(current_seq_2, s2[i]);  
    }

    int abs_class_1 = current_seq_1->abstract_class;
    int abs_class_2 = current_seq_2->abstract_class;

    if (abs_class_1 != -1 && abs_class_1 == abs_class_2) return 0;

    int abs_class_n = *p->abstract_classes_amount + 1;
    current_seq_1->abstract_class = abs_class_n;
    current_seq_2->abstract_class = abs_class_n;
    (*p->abstract_classes_amount)++;
    
    int n1_len = 0;
    char * name_1 = NULL;

    if (current_seq_1->abstract_class_name) {
        n1_len += (int)strlen(current_seq_1->abstract_class_name) + 1;
        name_1 = (char *)malloc(sizeof(char) * n1_len);
        
        if (name_1 == NULL) {
            errno = ENOMEM;
            return -1;
        }
        strcpy(name_1, current_seq_1->abstract_class_name);
    }

    int n2_len = 0;
    char * name_2 = NULL;
    
    if (current_seq_2->abstract_class_name) {
        n2_len += (int)strlen(current_seq_2->abstract_class_name) + 1;
        name_2 = (char *)malloc(sizeof(char) * n2_len);
        
        if (name_2 == NULL) {
            errno = ENOMEM;
            return -1;
        }
        strcpy(name_2, current_seq_2->abstract_class_name);
    }

    if (node_change(current_seq_1, name_1, name_2, abs_class_n) == -1) {
        return -1;
    }

    if (node_change(current_seq_2, name_1, name_2, abs_class_n) == -1) {
        return -1;
    }

    int zero_seqs =
        recur_equiv(
            p->next_zero, abs_class_1, abs_class_2, abs_class_n, name_1, name_2
        );

    int one_seqs =
        recur_equiv(
            p->next_one, abs_class_1, abs_class_2, abs_class_n, name_1, name_2
        );

    int two_seqs =
        recur_equiv(
            p->next_two, abs_class_1, abs_class_2,abs_class_n, name_1, name_2
        );

    if (name_1) free(name_1);
    if (name_2) free(name_2);
    
    if (zero_seqs != -1 && one_seqs != -1 && two_seqs != -1) return 1;
    else return -1;
}
