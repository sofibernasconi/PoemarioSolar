// Poemario Solar (Solar Poetry Device)
// Bilingual poem reader on a 2.13" e-ink display, powered by a solar-charged battery.
//
// Controls:
//   rotary encoder (turn)  -> next / previous page of the poem
//   rotary encoder (press) -> switch language (Spanish / English)
//   blue button            -> next poem
//   red switch             -> power on / off (cuts the 5V line, not in code)

#include <GxEPD2_BW.h>
#include <Adafruit_GFX.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMono9pt7b.h>

// ---------- E-INK ----------
static const int EPD_CS   = 10;
static const int EPD_DC   = 9;
static const int EPD_RST  = 8;
static const int EPD_BUSY = 7;

GxEPD2_BW<GxEPD2_213_B74, GxEPD2_213_B74::HEIGHT> display(
  GxEPD2_213_B74(EPD_CS, EPD_DC, EPD_RST, EPD_BUSY)
);

// ---------- ENCODER ----------
const int ENC_CLK = 2;
const int ENC_DT  = 3;
const int ENC_SW  = 4;

// ---------- BLUE BUTTON ----------
const int BLUE_BUTTON = 5;

int lastCLK;
int lastSW;
int lastButton = HIGH;

unsigned long lastStep = 0;
unsigned long lastButtonPress = 0;

const unsigned long debounceEncoder = 180;
const unsigned long debounceButton  = 250;

int screenIndex = 0;       // current screen inside the poem (0 = title)
bool englishMode = false;  // false = Spanish, true = English
int currentPoem = 0;

// ======================================================
// ================= DATA STRUCTURES ====================
// ======================================================

// One screen of text: a list of lines
struct Screen {
  const char** lines;
  int numLines;
};

// One poem: author, title in both languages, and its screens
struct Poem {
  const char* author;
  const char* title_es_1;
  const char* title_es_2;
  const char* title_en_1;
  const char* title_en_2;

  const Screen* screens_es;
  const Screen* screens_en;
  int totalScreens;
};

// The e-ink fonts do not include accents, so the text is written without them.
// Each screen fits about 7 lines of up to ~22 characters.

// ======================================================
// =============== ALEJANDRA PIZARNIK ===================
// ======================================================

const char* author_pizarnik = "Alejandra Pizarnik";

// Spanish
const char* pizarnik_1_es[] = {
  "poner aqui poema 1"
};

const Screen screens_pizarnik_es[] = {
  { nullptr, 0 }, // title
  { pizarnik_1_es, 1 }
};

// English
const char* pizarnik_1_en[] = {
  "poner aqui poema 1"
};

const Screen screens_pizarnik_en[] = {
  { nullptr, 0 }, // title
  { pizarnik_1_en, 1 }
};

// ======================================================
// ================= NICANOR PARRA ======================
// ======================================================

const char* author_parra = "Nicanor Parra";

// Spanish
const char* parra_1_es[] = {
  "poner aqui poema 2"
};

const Screen screens_parra_es[] = {
  { nullptr, 0 }, // title
  { parra_1_es, 1 }
};

// English
const char* parra_1_en[] = {
  "poner aqui poema 2"
};

const Screen screens_parra_en[] = {
  { nullptr, 0 }, // title
  { parra_1_en, 1 }
};

// ======================================================
// ================= SOFIA BERNASCONI ===================
// ======================================================

const char* author_sofia = "Sofia Bernasconi";

// Spanish
const char* quad1_es[] = {
  "Tu y yo escapamos del",
  "cuadrupedo invisible."
};

const char* quad2_es[] = {
  "No se si la noche",
  "habia llegado,",
  "el bosque denso no nos",
  "dejaba seguir",
  "nuestro sur."
};

const char* quad3_es[] = {
  "Mientras corriamos",
  "cuesta abajo",
  "comprendi que el",
  "cuadrupedo nunca",
  "existio"
};

const char* quad4_es[] = {
  "y que el sur",
  "siempre estuvo",
  "detras nuestro."
};

const Screen screens_sofia_es[] = {
  { nullptr, 0 }, // title
  { quad1_es, 2 },
  { quad2_es, 5 },
  { quad3_es, 5 },
  { quad4_es, 3 }
};

// English
const char* quad1_en[] = {
  "You and me escaped",
  "from the invisible",
  "quadruped."
};

const char* quad2_en[] = {
  "I dont know if the",
  "night had come,",
  "the dense forest",
  "wouldn't let us",
  "follow our south."
};

const char* quad3_en[] = {
  "As we ran down the",
  "steep slope",
  "I realized that the",
  "quadruped never",
  "existed"
};

const char* quad4_en[] = {
  "and the south had",
  "always been",
  "behind us."
};

const Screen screens_sofia_en[] = {
  { nullptr, 0 }, // title
  { quad1_en, 3 },
  { quad2_en, 5 },
  { quad3_en, 5 },
  { quad4_en, 3 }
};

// ======================================================
// ================= POEM LIST ==========================
// ======================================================

// totalScreens = number of text screens + 1 (the title screen)
const Poem poems[] = {
  {
    author_pizarnik,
    "LA ULTIMA", "INOCENCIA",
    "THE LAST", "INNOCENCE",
    screens_pizarnik_es,
    screens_pizarnik_en,
    2
  },
  {
    author_parra,
    "EL HOMBRE", "IMAGINARIO",
    "THE IMAGINARY", "MAN",
    screens_parra_es,
    screens_parra_en,
    2
  },
  {
    author_sofia,
    "EL CUADRUPEDO", "",
    "THE QUADRUPED", "",
    screens_sofia_es,
    screens_sofia_en,
    5
  }
};

const int totalPoems = sizeof(poems) / sizeof(poems[0]);

// ======================================================
// =================== DRAWING ==========================
// ======================================================

// Title screen: white text on black, with author and poem number
void showTitle(const char* line1, const char* line2, const char* author, int poemNumber, int totalPoems, bool inEnglish)
{
  display.setFullWindow();
  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_BLACK);
    display.setTextColor(GxEPD_WHITE);

    // Title
    display.setFont(&FreeMonoBold9pt7b);
    display.setCursor(8, 28);
    display.println(line1);

    if (strlen(line2) > 0) {
      display.setCursor(8, 50);
      display.println(line2);
    }

    // Author
    display.setFont(&FreeMono9pt7b);
    display.setCursor(8, 78);
    display.println(author);

    // Small footer
    display.setFont(NULL);
    display.setCursor(180, 110);

    if (inEnglish) {
      display.print("poem ");
    } else {
      display.print("poema ");
    }

    display.print(poemNumber);
    display.print("/");
    display.print(totalPoems);

  } while (display.nextPage());
}

// Text screen: black text on white, one line after another
void showBlock(const char* lines[], int numLines, int currentPage, int totalInnerPages)
{
  display.setFullWindow();
  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);
    display.setFont(&FreeMono9pt7b);

    int y = 16;
    int lineSpacing = 17;

    for (int i = 0; i < numLines; i++) {
      display.setCursor(8, y);
      display.println(lines[i]);
      y += lineSpacing;
    }

    // // Page number inside the poem
    // display.setFont(NULL);
    // display.setCursor(220, 110);
    // display.print(currentPage);
    // display.print("/");
    // display.print(totalInnerPages);

  } while (display.nextPage());
}

int maxScreenIndex()
{
  return poems[currentPoem].totalScreens - 1;
}

// Draws the current screen of the current poem in the current language
void showCurrent()
{
  const Poem& poem = poems[currentPoem];

  if (screenIndex == 0) {
    if (englishMode) {
      showTitle(
        poem.title_en_1,
        poem.title_en_2,
        poem.author,
        currentPoem + 1,
        totalPoems,
        true
      );
    } else {
      showTitle(
        poem.title_es_1,
        poem.title_es_2,
        poem.author,
        currentPoem + 1,
        totalPoems,
        false
      );
    }
    return;
  }

  const Screen* screenList = englishMode ? poem.screens_en : poem.screens_es;

  showBlock(
    screenList[screenIndex].lines,
    screenList[screenIndex].numLines,
    screenIndex,
    poem.totalScreens - 1
  );
}

// ======================================================
// ==================== SETUP ===========================
// ======================================================

void setup()
{
  Serial.begin(115200);

  pinMode(ENC_CLK, INPUT_PULLUP);
  pinMode(ENC_DT, INPUT_PULLUP);
  pinMode(ENC_SW, INPUT_PULLUP);
  pinMode(BLUE_BUTTON, INPUT_PULLUP);

  lastCLK = digitalRead(ENC_CLK);
  lastSW  = digitalRead(ENC_SW);

  display.init(115200);
  display.setRotation(3);

  showCurrent();
}

// ======================================================
// ===================== LOOP ===========================
// ======================================================

void loop()
{
  // ---------- BLUE BUTTON: NEXT POEM ----------
  int buttonState = digitalRead(BLUE_BUTTON);

  if (lastButton == HIGH && buttonState == LOW) {
    delay(30);

    if (digitalRead(BLUE_BUTTON) == LOW) {
      currentPoem++;
      if (currentPoem >= totalPoems) currentPoem = 0;

      screenIndex = 0;
      showCurrent();
    }
  }

  lastButton = buttonState;

  // ---------- ENCODER TURN: PAGE NAVIGATION ----------
  int currentCLK = digitalRead(ENC_CLK);

  if (lastCLK == HIGH && currentCLK == LOW) {
    unsigned long now = millis();

    if (now - lastStep > debounceEncoder) {
      int maxIndex = maxScreenIndex();

      if (digitalRead(ENC_DT) == HIGH) {
        screenIndex--;
        if (screenIndex < 0) screenIndex = maxIndex;
      } else {
        screenIndex++;
        if (screenIndex > maxIndex) screenIndex = 0;
      }
      showCurrent();
      lastStep = now;
    }
  }

  lastCLK = currentCLK;

  // ---------- ENCODER PRESS: SWITCH LANGUAGE ----------
  int currentSW = digitalRead(ENC_SW);

  if (lastSW == HIGH && currentSW == LOW) {
    unsigned long nowPress = millis();

    if (nowPress - lastButtonPress > debounceButton) {
      englishMode = !englishMode;
      showCurrent();
      lastButtonPress = nowPress;
    }
  }

  lastSW = currentSW;
}
