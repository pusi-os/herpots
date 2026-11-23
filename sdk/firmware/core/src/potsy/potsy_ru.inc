// PotSquad RU chatter. Display-only parallels to the EN banks in potsy.cpp:

// same counts, same order, picked by the same roll. ASCII + Cyrillic only

// (no «»/--, the RU face has neither). Ported from BroWatch; IDLE/ONBOARD

// rebranded сквач->потси, DET index 5 rewritten for RAVEN, HINTS extended

// for the Pot-only eggs, Pot-only banks written fresh.

static inline bool isRU() { return Settings::lang() == 1; }

// Size of a pool as compiled. The RU banks are trimmed independently of the
// EN ones they mirror, so every pick must read its own array's length rather
// than a number that could outlive an edit.
#define POOL_N(a) (static_cast<int>(sizeof(a) / sizeof((a)[0])))

static const char* pickL(const char* const* en, const char* const* ru, int n) {

    const int i = random(0, n);

    return isRU() ? ru[i] : en[i];

}

static const char* ALERT_MOOD_LINES_RU[] = {
    "Держу ухо востро. День бурный.",
    "Смена жаркая. Не жалуюсь.",
};

static const char* BOOT_LINES_RU[] = {
    "БроВотч в эфире. Поищем чего.",
    "Загрузился. Не пялься в телефон.",
};

static const char* BORED_LINES_RU[] = {
    "...тут есть кто?",
    "Прогулялся бы. Просто говорю.",
};

static const char* DANCE_LINES_RU[] = {
    "Никто не смотрит. Кроме тебя.",
    "Это моё лучшее движение.",
};

static const char* const DUCK_LINES_RU[] = {
    "Тостер чуть не достал!",
    "Этот летел прямо в меня.",
};

static const char* const EARLY_LINES_RU[] = {
    "Рано встал. Или не ложился. Привет.",
    "Сначала кофе. Потом квартал.",
};

static const char* ENCOURAGE_LINES_RU[] = {
    "Пусто? Скучно. Иди гуляй.",
    "Выведи меня, я нервничаю.",
};

static const char* const EVENING_LINES_RU[] = {
    "Вечер. Коммутеры зажигают эфир.",
    "Где-то конец дня. Не тут. Тут скан.",
};

static const char* const FRIDAY_LINES_RU[] = {
    "Пятница. Даже Флоки устали.",
    "Пятница. Вези меня где WiFi хуже.",
};

static const char* const GUM_LINES_RU[] = {
    "Смотри сюда.",
    "Жвачка. Штатная.",
};

static const char* HELD_LINES_RU[] = {
    "Ладно, это правда приятно.",
    "Не останавливайся. Серьёзно.",
};

static const char* const HUNT_COLDER_LINES_RU[] = {
    "Холоднее. В другую сторону?",
    "Не-а. Не туда, чемпион.",
};

static const char* const HUNT_FIRST_SIGNAL_LINES_RU[] = {
    "О, вот оно!",
    "Есть сигнал. Пошли.",
};

static const char* const HUNT_HOT_LINES_RU[] = {
    "Ты почти стоишь на нём.",
    "Так близко и всё ищешь? Смело.",
};

static const char* const HUNT_STALLED_LINES_RU[] = {
    "Всё ещё тут? Уважаю. Или упрямство.",
    "Давно тут стоим, шеф.",
};

static const char* const HUNT_WARMER_LINES_RU[] = {
    "Теплее!",
    "Да, туда.",
};

static const char* const JUGGLE_LINES_RU[] = {
    "Гляди, что поймал.",
    "Три сразу. Легко.",
};

static const char* const LATE_LINES_RU[] = {
    "Глубокая ночь. Спи. Я подежурю.",
    "После полуночи в Bluetooth добра нет.",
};

static const char* LOG_CLEAR_LINES_RU[] = {
    "Журнал чист. Начнём по-криптидски.",
};

static const char* LOG_OPEN_LINES_RU[] = {
    "Роемся в журнале? Смело.",
    "Тут живут все пруфы.",
};

static const char* const LUNCH_LINES_RU[] = {
    "Обед. Мне ничего, я экран.",
    "Полквартала прошло с Bluetooth.",
};

static const char* const MEET_GUEST_LINES_RU[] = {
    "Слышал, тут кто-то смотрит.",
    "Третьим будете?",
};

static const char* const MEET_HOST_LINES_RU[] = {
    "О — гости.",
    "Ну надо же. Кто нас нашёл.",
};

static const char* const MONDAY_LINES_RU[] = {
    "Понедельник. Камерам плевать, а мне нет.",
    "Понедельник. Телефоны на работе, трекеры тоже.",
};

static const char* const MORNING_LINES_RU[] = {
    "Утренняя смена. По нулям везде.",
    "Фургоны доставки — самое громкое в эфире.",
};

static const char* const NIGHT_LINES_RU[] = {
    "Позднеет. Трекеры не спят, а ты спи.",
    "Ночная смена. Только я и лампочки.",
};

static const char* const NOIR_LINES_RU[] = {
    "Галопирующие таблицы. Опять ничего.",
    "Видал такое — не поверишь. В основном принтеры.",
    "Шипящие подстанции, эфир тих. Не верю.",
};

static const char* const OATH_LINES_RU[] = {
    "Святые хотспоты. %s.",
    "Очуметь, оптика! %s. Веди себя спокойно.",
};

static const char* PARTY_LINES_RU[] = {
    "ТУСА-РЕЖИМ. Пожалуйста.",
    "Ого. Видал это?",
};

static const char* const PART_GUEST_LINES_RU[] = {
    "Ладно, за дело.",
    "Глаз не закрывай.",
};

static const char* const PART_HOST_LINES_RU[] = {
    "Там потише.",
    "Глаз да глаз.",
};

static const char* PERSISTENT_LINES_RU[] = {
    "Этот возвращается. На заметку.",
    "Уже не случайность. Приглядывай.",
};

static const char* PETTING_LINES_RU[] = {
    "Да. Вот это тема.",
    "Не говорю, что мурчу. И обратного нет.",
};

static const char* PET_LINES_RU[] = {
    "О, вот туда.",
    "Личное пространство? Не слышал.",
    "Гладишь криптида. Смело.",
    "Потому фото мои и мутные.",
    "Ладно, ЕЩЁ раз. Другим ни слова.",
};

static const char* RELAXED_MOOD_LINES_RU[] = {
    "Тихо — можно спать стоя.",
    "Сегодня одни вайбы.",
};

static const char* ROTATE_LINES_RU[] = {
    "Эй, полегче с вращениями.",
    "Вот это поворот.",
};

static const char* const SCAN_EMPTY_LINES_RU[] = {
    "...пусто? Хм.",
    "Тихо сегодня.",
};

static const char* const SCAN_FOUND_LINES_RU[] = {
    "Сигналов-то сколько.",
    "Оживлённый район!",
};

static const char* const SCAN_HIT_LINES_RU[] = {
    "О, нашёл!",
    "Есть попадание.",
};

static const char* const SCARE_LINES_RU[] = {
    "Это по нам?", "Ты видел?", "Я почуял.", "Опять.",
};

static const char* SEEN_BEFORE_LINES_RU[] = {
    "Этого уже видел.",
    "Снова встретились.",
};

static const char* SLEEPY_LINES_RU[] = {
    "*зевок* ...я тут.",
    "Криптид-дрема. Никому ни слова.",
};

static const char* const SLUMP_LINES_RU[] = {
    "Три часа дня. Зевают даже роутеры.",
    "После обеда. Движутся лишь номера.",
};

static const char* const STRETCH_LINES_RU[] = {
    "Нннгх. Ладно. Встал.",
    "Пять минут ещё. ...Ладно.",
};

static const char* WALK_LINES_RU[] = {
    "Разминаю лапы.",
    "Время патруля.",
};

static const char* WATCHING_LINES_RU[] = {
    "Смотрю. Мимо ничего.",
    "Глаза открыты. Ты под крылом.",
    "Сканирую эфир. Пока тихо.",
    "Я на стрёме. Занимайся своим.",
};

static const char* const WATCH_ALERT_LINES_RU[] = {
    "Говорил же.",
    "Я знал, что вернётся.",
};

static const char* const WEEKEND_LINES_RU[] = {
    "Выходные. Телефоны района дома.",
    "Субботний скан. Выходной криптида — скан другого.",
};

static const char* const WINK_LINES_RU[] = {
    "О — не видел тебя.",
    "Да, я знаю, что смотришь.",
};



static const char* const DAY_MILESTONES_RU[] = {
    /* 1000 */ "Тысяча дней. Это куча Флоков.",
    /*  730 */ "Два года сегодня. Так и не поймали.",
    /*  500 */ "Пятьсот дней. Забыл, как выглядит коробка.",
    /*  365 */ "Год. Целый год. Никто не поймал.",
    /*  200 */ "Двести дней вместе. Купил бы торт, да держать нечем.",
    /*  100 */ "Сто дней. Шлите торт. Или батарейки.",
    /*   30 */ "Месяц. Этот стол знаю лучше своей пещеры.",
    /*    7 */ "Неделя с тобой. Пока не поймали.",
    /*    1 */ "Второй день вместе. Перезагрузки считать перестал.",
};

static const char* const FRIEND_HELLOS_RU[] = {
    "Снова ты!", "Любимый гость.", "Опять ты? Хорошо.", "Опять на месте?",
};

static const char* const RPS_CALLS_RU[] = {
    "Камень, ножницы, бумага!", "До одной. Погнали!", "Камень, ножницы, БЕЙ!", "Решим.",
};

static const char* const RPS_WIN_RU[]  = { "Непобедим.", "Слишком просто.", "Читаю тебя как книгу." };
static const char* const RPS_LOSE_RU[] = { "До трёх?", "Подстроено.", "Я поддался." };
static const char* const RPS_TIE_RU[]  = { "Великие умы.", "Ещё. Ещё.", "Сглазили." };
static const char* const SNOW_CALLS_RU[] = { "Лови!", "Берегись!", "Подача!", "Хватай!" };
static const char* const SNOW_REPLIES_RU[] = {
    "О, началось.", "Ты за это ответишь.", "Холодно! ХОЛОДНО!", "Моя очередь.",
};

static const char* const DANCE_CALLS_RU[] = {
    "Танцы. Ща.", "Переплюнь.", "Смотри на ноги.", "Попробуй успей.",
};

static const char* const DANCE_REPLIES_RU[] = {
    "Держи мой мех.", "О, началось.", "Час любителя.", "Моя очередь.",
};

static const char* const BG_LINES_RU[][3] = {
    /* DIGITAL   */ { "Опять цифровой дождь. По-хакерски.", "Падающий код, бурая шерсть.", "Прочитал бы, да лень." },
    /* STARFIELD */ { "Звёзды. Космически.", "Где-то там криптид покрупнее.", "Космос — тот же лес, темнее." },
    /* TOASTERS  */ { "Летающие тостеры. Классика.", "Столько тостов в небе не надо.", "Сегодня вайб After Dark." },
    /* AQUARIUM  */ { "Аквариум. Дзен.", "Рыбы без конспирации. Новички.", "Аквариум завёл бы, но камер стесняюсь." },
    /* TERMINAL  */ { "Терминал. Мой темп.", "Зелёный текст, бурая шерсть.", "Выглядит официально. Вайбы в основном." },
    /* FIREFLIES */ { "Светлячки сегодня. Красота.", "Огоньки, атмосфера.", "Они не следят. Проверял." },
    /* FIRE      */ { "Огонь на фоне. Уютно.", "Тепло, без пожарных.", "Ничего не горит. Наверное." },
    /* SNOWFALL  */ { "Опять снег. Лапам сцепление.", "Погода для таинственных следов.", "Холодно. Я к такому готов." },
    /* SPECTRUM  */ { "Спектр живой. Вот это дело.", "Это настоящие сигналы. Круто, да?", "Слушаю эфир. Очень в стиле." },
    /* TUNNEL    */ { "Туннель. Ретрофутуризм.", "Будто едем куда-то. Нет.", "Вайбы sci-fi 80-х." },
    /* SYNTHWAVE */ { "Закат не садится. Проверял.", "Сетка бесконечна. Как и путь.", "Глянь на отражение. Водичка-то что." },
    /* BLACK     */ { "Стен пустых сегодня.", "Мы и темнота. Мне норм.", "Фаза минимализма. Бывает." },
};

static const char* IDLE_LINES_RU[] = {
    "Оставайся потси.",
    "Живу в этих стенах годами.",
    "Не будь скриптом. Учись.",
    "Этот WiFi наводит на мысли.",
    "Я везде и нигде.",
};

static const char* ONBOARD_LINES_RU[] = {
    "Привет! Я Потси. Две минуты — и отпущу.",
    "ПотСквад слышит слежку: камеры, номера, трекеры.",
    "Без магии: WiFi и Bluetooth сверяем с базой.",
    "Нули внизу — всё чисто. При находке будет ALERT.",
    "Внизу: СКАН ищет, ЖУРНАЛ помнит, ЧАСЫ тикают.",
    "Сверху слева настройки, края экрана меняют фон.",
};

static const DetLines DET_LINES_RU[] = {
    { "Что-то рядом.",                     "Сигнал не опознан. На стрёме." },  // UNKNOWN
    { "Флок засёк. Старший Брат машет.",   "Камера номеров. Ты в каталоге." }, // FLOCK
    { "Рядом Axon. Веди себя прилично.",   "Нагрудная камера. Улыбнись." },    // AXON
    { "Очки стучат. Дикие времена.",       "Чьи-то очки записывают." },        // META
    { "Скиммер! Тут не плати.",            "Грубый синезубый прибор." },       // SKIMMER
    { "Меш-узел в эфире. Тоже сам по себе.", "Меш-радио где-то рядом. Брат." }, // MESH
    { "Рядом AirTag. Надеюсь, твой.",      "Что-то что-то отслеживает." },     // AIRTAG
    { "Глаза в небе. Буквально.",          "Дрон вверху. Маши, если готов." }, // DRONE
    { "Читалка номеров. Классика.",        "Номера видят тебя. Улыбнись." },   // ALPR
    { "Камера засекла. Улыбнись, легенда.","Кто-то смотрит. Выгляди." },       // CAMERA
    { "Метка Samsung. Кто-то метит.",      "Galaxy SmartTag рядом. Хм." },     // SAMSUNG_TAG
    { "Сеть слежки Google машет.",         "Find My? Это я нашёл." },          // GOOGLE_TAG
    { "Метка Tile. Надеюсь, друг.",        "Кроха что-то отслеживает." },      // TILE
    { "Звонок Ring. Улыбнись Амазону.",    "Чей-то звонок тебя судит." },      // RING
};




static const char* const BACK_LINES_RU[] = {
    "Ближе. Мне не нравится ближе.",
    "Ладно. Личное пространство.",
};


static const char* const CHECKING_LINES_RU[] = {
    "Смотришь опять. Я бы сказал.",
    "Пятый раз за минуту. Без изменений.",
};


static const char* const CLOSEST_LINES_RU[] = {
    "Ближайший %s. Дышал мне в ухо.",
    "Рекорд. Этот %s был прямо в шерсти.",
};


static const char* const DIZZY_LINES_RU[] = {
    "Где тут лес?",
    "Я слышу цвета.",
};


static const char* const FLICK_LINES_RU[] = {
    "ЭЙ!",
    "Грубо.",
};


static const char* const IGNORED_LINES_RU[] = {
    "Сутки без глажки. Я в порядке. Точно.",
    "24 часа. Ни одной глажки. Считаю.",
};


static const char* const NEMESIS_LINES_RU[] = {
    "%s. Мой старый враг.",
    "Опять ты, %s.",
};


static const char* const NEW_REGULAR_LINES_RU[] = {
    "Три дня подряд. Зову тебя %s.",
    "Ладно, ты свой. Привет, %s.",
};


static const char* const REGULAR_LINES_RU[] = {
    "Снова %s. Точно в срок.",
    "О, это просто %s.",
};


static const char* const RUN3_LINES_RU[] = {
    "Третий %s за десять минут. Ходят стаями.",
    "Это уже три %s. Кто-то их собирает.",
};


static const char* const RUN5_LINES_RU[] = {
    "Пять %s. Это съезд %s.",
    "Пять. Пять %s. Нужен журнал побольше.",
};


static const char* const RUN10_LINES_RU[] = {
    "Десять %s. Я сбился. Не сбился.",
};


static const char* const LAND_LINES_RU[] = {
    "Приземлился.",
    "От судей? Четыре.",
};


static const char* const LEAN_LINES_RU[] = {
    "Слышал?",
    "Стоп. Там что-то есть.",
};


static const char* const SHAKE_LINES_RU[] = {
    "Фу. Сними это.",
    "Липкое какое-то.",
};


static const char* const SIT_LINES_RU[] = {
    "Передышка.",
    "Ничего нет. Сижу.",
};


static const char* const THROW_LINES_RU[] = {
    "УИИИИИИ!",
    "Не ТАК!",
};


static const char* const TICKLE_HEAD_LINES_RU[] = {
    "Гладь по голове. Приемлемо.",
    "Вот туда. Да.",
};


static const char* const TICKLE_BELLY_LINES_RU[] = {
    "Хи. Хватит. Не останавливайся.",
    "Щекотно! ЩЕКОТНО!",
};


static const char* const TICKLE_FEET_LINES_RU[] = {
    "Не ноги!",
    "ЭЙ. Ноги - личное.",
};


static const char* const TRIP_LINES_RU[] = {
    "Кто это поставил?",
    "Я так и хотел.",
};




// Indexed pick: the same seed reads the same slot in either language.
static const char* selL(const char* const* en, const char* const* ru,
                        uint8_t n, uint32_t seed) {
    const uint8_t i = (uint8_t)(seed % n);
    return isRU() ? ru[i] : en[i];
}
