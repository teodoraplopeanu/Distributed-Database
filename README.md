
# Distributed Database

## Descriere:

* Tema implementeaza un sistem distribuit, a carui memorie cache functioneaza pe principiul LRU (Least Recently Used), iar balancing-ul serverelor si al documentelor se face pe baza Consistent Hashing.

## Load Balancer

* Load balancer-ul contine un hash_ring pe care sunt amplasate serverele, construit sub forma unei liste circulare dublu inlantuite.
* Pe aceste servere sunt distribuite documente, conform Consistent Hashing. Cu alte cuvinte, daca hash-ul unui document se afla, pe acest hash_ring imaginar, intre hash-ul serverulu X si hash-ul serverului Y, daca acestea sunt ordonate crescator, atunci documentul va apartine serverului Y.
* Atunci cand un nou server este adaugat in retea sau un altul este scos, se fac schimbarile necesare astfel incat regula distributiei documentelor sa se pastreze.


## Server
Serverul are in structura sa:
* un database, unde sunt stocate toate documentele
* o memorie LRU cache, unde sunt stocate documentele cel mai recent utilizate 
(componenta ei se schimba in mod constant)
* cache_capacity
* id - cu care este identificat pe hash ring
* o coada cu task-uri ce urmeaza a fi executate la urmatoarea comanda GET

### Functionare:
* Atunci cand serverul primeste o comanda EDIT, o adauga in coada de task-uri
* Cand primeste o comanda GET, executa toate edit urile din coada, si abia la final GET-ul

## LRU Cache
* LRU Cache are in componenta sa o lista circulara dublu inlantuita (unde un nod este o structura de tip doc_t) si un hashmap (unde cheia este numele documentului si valoarea nodul corespunzator din dll), precum si cache_capacity.
* Atunci cand adaugam un nou element in cache, verificam mai intai daca acesta este plin. Daca da, atunci eliminam intrarea cea mai veche (cea de pe list->size - 1) si ii returnam numele documentului (cel eliminat) cu ajutorul evicted_key, pentru a il afisa la response.

* Interactiunea cu utilizatorul se face pe baza unui response, ce se alcatuieste cu ajutorul unor macro-uri, in care adaugam, cu ajutorul functiei sprintf, argumentele corespunzatoare.

