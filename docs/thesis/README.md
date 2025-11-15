# 📄 DOCUMENTAȚIE ACADEMICĂ - LICENȚĂ

## Proiect: Framework REST API în C++ cu Multi-Processing și IPC

**Student:** Eminint
**Instituție:** Academia Tehnică Militară "Ferdinand I"
**An:** 2025

---

## 📋 Status Documentație

### ✅ Capitole Completate

- [x] **Capitolul 1: Introducere** (CAPITOLUL_1_INTRODUCERE.md)
  - Context și motivație
  - Problema identificată
  - Obiective (general + 7 obiective specifice)
  - Contribuții (științifice, tehnice, aplicative)
  - Organizarea lucrării
  - **Pagini: 3-4**

- [x] **Capitolul 2: Related Work - Studiul Literaturii** (CAPITOLUL_2_RELATED_WORK.md)
  - Framework-uri în alte limbaje (Express.js, FastAPI, Gin, Actix-web)
  - Framework-uri C++ (Crow, Pistache, Drogon, cpp-httplib)
  - Concepte teoretice (Multi-processing, IPC, Threading, I/O non-blocking, Signals)
  - Gap analysis
  - **Pagini: 5-6**

### 🔄 Capitole În Lucru

- [ ] **Capitolul 3: Arhitectură și Design**
  - Cerințe funcționale și non-funcționale
  - Arhitectura de nivel înalt (3-layer)
  - Arhitectura multi-processing (Master/Worker)
  - IPC architecture (SharedMemory, SharedQueue, Semaphores)
  - Diagrame UML (Component, Deployment, Sequence, Class)
  - **Pagini: 7-9**
  - **DIAGRAME NECESARE:**
    - Component Diagram (3 layers)
    - Deployment Diagram (Master + Workers)
    - Sequence Diagram (IPC communication)
    - Sequence Diagram (HTTP request flow)
    - Class Diagram (ThreadPool)
    - State Machine (Graceful shutdown)

- [ ] **Capitolul 4: Implementare**
  - Tehnologii utilizate (C++17, CMake, POSIX, SQLite)
  - Structura proiectului (tree diagram)
  - Detalii implementare Master Process
  - Detalii implementare Worker Process
  - Detalii IPC (shm_open, mmap, semafoare)
  - Detalii ThreadPool
  - Detalii HTTP Parser și Router
  - **Pagini: 8-10**

- [ ] **Capitolul 5: Testare și Validare** ⚠️ **CRITIC**
  - Metodologie de testare
  - Environment de testare
  - Unit tests (Router, SharedQueue, ThreadPool)
  - Integration tests (Master-Worker)
  - **Performance benchmarks** (TREBUIE RULATE!)
    - Throughput vs Workers (1, 2, 4, 8)
    - Latency distribution (P50, P95, P99)
    - Concurrency test (10, 100, 1000 connections)
  - **Comparative benchmarks** (vs Crow, Pistache)
  - Validare use-cases
  - **Pagini: 8-10**
  - **GRAFICE NECESARE:**
    - Scalability graph (workers vs req/sec)
    - Latency histogram
    - Concurrency vs latency
    - Comparative bar charts

- [ ] **Capitolul 6: Concluzii și Perspective**
  - Rezultate obținute
  - Contribuții (sinteză)
  - Limitări actuale
  - Direcții viitoare (WebSocket, HTTP/2, TLS, Kubernetes)
  - Impact și aplicabilitate
  - **Pagini: 3-4**

- [ ] **Bibliografie**
  - Minim 20 referințe
  - Format IEEE sau APA
  - Categorii:
    - Cărți (Stallings, Silberschatz, Kerrisk, Stevens, Stroustrup)
    - Articole științifice
    - Documentație tehnică (POSIX, HTTP RFC)
    - Framework-uri studiate (Express, FastAPI, Crow, etc.)
  - **Pagini: 2-3**

- [ ] **Anexe** (opțional)
  - Anexa A: Fragmente de cod relevante
  - Anexa B: Configurații de testare
  - Anexa C: Grafice suplimentare
  - Anexa D: Manual de utilizare
  - **Pagini: 5-10**

---

## 📊 Progres Total

**Pagini completate:** 8-10 / 30-40 (≈25%)
**Capitole completate:** 2 / 6 (33%)

### Estimare timp rămas:

| Task | Timp estimat |
|------|--------------|
| Capitolul 3 (Arhitectură + Diagrame) | 1-2 zile |
| Capitolul 4 (Implementare) | 1 zi |
| Capitolul 5 (Testare + Benchmarks) | 2-3 zile |
| Capitolul 6 (Concluzii) | 0.5 zile |
| Bibliografie | 0.5 zile |
| Review și formatare | 1 zi |
| **TOTAL** | **6-9 zile** |

---

## 🎯 Următorii Pași Critici

### Prioritate MAXIMĂ

1. **Rulează benchmark-urile de performanță**
   ```bash
   cd build
   ./benchmark_throughput > results_throughput.txt
   ```
   **DE CE:** Capitolul 5 necesită rezultate reale, nu estimări!

2. **Creează diagramele pentru Capitolul 3**
   - Folosește draw.io, Lucidchart, sau PlantUML
   - Export la PNG/PDF pentru inclus în document

3. **Scrie Capitolul 3 (Arhitectură și Design)**
   - Include toate diagramele
   - Explică deciziile de design

4. **Scrie Capitolul 5 (Testare și Validare)**
   - Include rezultatele reale din benchmarks
   - Creează tabele și grafice

---

## 📝 Instrucțiuni de Formatare

### Pentru LaTeX (recomandat pentru lucrare de licență):

1. Instalează LaTeX:
   ```bash
   sudo apt-get install texlive-full
   ```

2. Convertește Markdown → LaTeX:
   ```bash
   pandoc CAPITOLUL_1_INTRODUCERE.md -o CAPITOLUL_1.tex
   ```

3. Compilează documentul final:
   ```bash
   pdflatex main.tex
   bibtex main
   pdflatex main.tex
   pdflatex main.tex
   ```

### Pentru Word/LibreOffice:

1. Convertește Markdown → DOCX:
   ```bash
   pandoc CAPITOLUL_1_INTRODUCERE.md -o CAPITOLUL_1.docx
   ```

2. Deschide în Word/LibreOffice și aplică stiluri

---

## 📐 Cerințe de Formatare (Standardizate)

- **Font:** Times New Roman 12pt (text), 14pt (titluri)
- **Spacing:** 1.5 lines
- **Margins:** 2.5cm stânga, 1.5cm dreapta, 2cm sus/jos
- **Numerotare pagini:** Jos-dreapta
- **Figuri și tabele:** Numerotate și cu caption
- **Cod:** Courier New 10pt, fundal gri deschis
- **Referințe:** Numerotare [1], [2], etc.

---

## 🔧 Tools Recomandate

### Pentru Diagrame:
- **draw.io** (https://app.diagrams.net) - Gratis, online
- **Lucidchart** (https://www.lucidchart.com) - Online cu trial
- **PlantUML** - Text-based UML diagrams
- **Microsoft Visio** - Dacă ai licență

### Pentru Grafice (Benchmark Results):
- **Microsoft Excel** - Tabele + grafice
- **LibreOffice Calc** - Open-source alternative
- **Python matplotlib** - Pentru grafice programatice
- **Gnuplot** - Pentru grafice din command line

### Pentru Bibliografie:
- **Zotero** (https://www.zotero.org) - Free bibliography manager
- **Mendeley** - Alternative
- **BibTeX** - Dacă folosești LaTeX

---

## ✅ Checklist Pre-Prezentare

### Documentație:
- [ ] Toate cele 6 capitole scrise
- [ ] Minim 6 diagrame create
- [ ] Minim 20 referințe în bibliografie
- [ ] Format profesional aplicat
- [ ] PDF generat și verificat
- [ ] Fără greșeli gramaticale

### Teste și Validare:
- [ ] Unit tests rulate și trecute
- [ ] Benchmarks rulate cu rezultate concrete
- [ ] Rezultate exportate în tabele
- [ ] Grafice create din rezultate

### Prezentare:
- [ ] Slide-uri prezentare pregătite (10-15 slide-uri)
- [ ] Demo script testat și funcțional
- [ ] Răspunsuri pregătite la întrebări comune

---

## 📞 Întrebări Frecvente

**Î: Câte pagini trebuie să fie lucrarea?**
**R:** 25-40 pagini (fără anexe). Cu anexe poate ajunge la 50 pagini.

**Î: Trebuie să fie în LaTeX obligatoriu?**
**R:** Nu, dar LaTeX arată mai profesional. Word/LibreOffice este acceptat.

**Î: Pot folosi figuri din documentația altora?**
**R:** DA, dar trebuie citate sursa în caption. Ex: "Figura 2.1: Arhitectura Express.js [Sursa: expressjs.com]"

**Î: Trebuie să iau permisiune pentru benchmark-uri comparative?**
**R:** Nu, benchmark-urile sunt considerate "fair use" pentru cercetare academică.

**Î: Cât de detaliat trebuie să fie Capitolul 5 (Testare)?**
**R:** FOARTE detaliat! Trebuie tabele cu rezultate concrete, grafice, și interpretare.

---

## 📧 Contact

Pentru întrebări despre documentație:
- Verifică README-urile din:
  - `/tests/` pentru teste
  - `/demo/` pentru demonstrații
  - `/benchmarks/` pentru benchmark-uri

---

**Ultima actualizare:** 15 Noiembrie 2025
**Versiune document:** 1.0 (Draft)
