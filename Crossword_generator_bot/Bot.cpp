#include <stdio.h>
#include "Generator.h"
#include <tgbot/tgbot.h>
#include <unordered_map>
#include <map>
#include <sqlite3.h>

long long users_count = -1;

struct User
{
    bool language_flag = true;
    bool number_input_flag = false;
    bool word_input_flag = false;
    int word_num = 0;
    bool blocked = false;
    std::string entered_words;
    std::vector<std::string> words;

};

wchar_t Rus_upper(wchar_t sim)
{
    if (sim >= L'а' && sim <= L'я')
        return (sim - 32);
    else if (sim == L'ё')
        return L'Ё';
    else if (sim == L'й')
        return L'Й';
    return sim;
}

void IncorrectMessages(TgBot::Bot& bot, TgBot::Message::Ptr& message, const std::string& mes, User& user)
{
    std::string message_text;
    if (user.language_flag)
        message_text = u8" Use /instruction for information on how to use the bot. 🔧";
    else
        message_text = u8" Используйте команду /instruction, для получения информации о боте. 🔧";

    bot.getApi().sendMessage(message->chat->id, mes + message_text);
}

void Instruction(TgBot::Bot& bot, TgBot::Message::Ptr& message, User& user)
{
    std::string instruction_text;
    if (user.language_flag)
        instruction_text = u8"🔸 To start, enter the /start command and select the language.\n"
        u8"🔸 To get instructions for use, enter /instruction. 🔧\n"
        u8"🔸 To start generating the crossword, type /generate.\n"
        u8"Then you need to enter the number of words that the crossword will consist of. (2 - 20)\n"
        u8"Then enter the words, one at a time in the message.\nThe word length can be no more than 20 characters.\n"
        u8"A word can consist only of the letters of the alphabet of the language you have chosen.\n"
        u8"After making sure that the entered words are correct, click \"YES\" and expect the result.\n"
        u8"(up to 5 minutes) ⏰\n"
        u8"⭐ Have a good use! ⭐";
    else
        instruction_text = u8"🔸 Используйте команду /start, для начала работы и выбора языка.\n"
        u8"🔸 Используйте команду /instruction, для получения информации о боте. 🔧\n"
        u8"🔸 Для того чтобы приступить к генерации кроссворда, используйте команду /generate.\n"
        u8"Затем введите число слов, из которых будет состоять кроссворд. (2 - 20)\n"
        u8"Затем введите слова, по одному в сообщении.\nДлина слова не более 20 символов.\n"
        u8"Слово может состоять только из букв алфавита выбранного вами языка.\n"
        u8"После убедитесь в правильности введенной информации и нажмите \"YES\". Ожидайте результат."
        u8"\n(занимает до 5 минут) ⏰\n"
        u8"⭐ Приятного использования! ⭐";

    bot.getApi().sendMessage(message->chat->id, instruction_text);
}

void MakeTwoButtonsKeyboard(TgBot::InlineKeyboardMarkup::Ptr& keyboard, const std::string& b1, const std::string& b2)
{
    std::vector<TgBot::InlineKeyboardButton::Ptr> buttons{ std::make_shared<TgBot::InlineKeyboardButton>(), std::make_shared<TgBot::InlineKeyboardButton>() };
    buttons[0]->text = b1;
    buttons[0]->callbackData = b1;
    buttons[1]->text = b2;
    buttons[1]->callbackData = b2;
    keyboard->inlineKeyboard.push_back(buttons);
}

bool CheckWord(TgBot::Message::Ptr& message, User& user)
{
    std::string message_text = message->text;
    if (message_text.size() > 20 || message_text.size() < 2 || message_text.find(u8' ') != std::string::npos)
        return false;

    if (user.language_flag)
    {
        for (auto& sim : message_text)
        {
            if (!(std::toupper(sim) >= u8'A' && std::toupper(sim) <= u8'Z'))
                return false;
        }
    }
    else
    {
        for (auto& sim : FromUtf8(message_text))
        {
            if (!((Rus_upper(sim) >= L'А' && Rus_upper(sim) <= L'Я') || Rus_upper(sim) == L'Й' || Rus_upper(sim) == L'Ё'))
                return false;
        }
    }
    return true;
}

void BadEnter(User& user)
{
    user.words.clear();
    user.word_num = 0;
    user.word_input_flag = false;
}

void GenerateCrossword(std::vector<std::string>& words, TgBot::Bot& bot, int64_t id, User& user)
{
    user.blocked = true;

    bot.getApi().sendVideo(id, TgBot::InputFile::fromFile("giphy.gif.mp4", "video/mp4"));

    Generator g;
    g.Generate(words);

    if (user.language_flag)
        bot.getApi().sendMessage(id, u8"Generation completed.");
    else
        bot.getApi().sendMessage(id, u8"Генерация завершилась.");

    std::string crossword = g.GetCrossword(user.language_flag);

    std::ofstream file("crossword.rtf");
    file << crossword;
    file.close();

    file.open("crossword.html");
    file << crossword;
    file.close();

    bot.getApi().sendDocument(id, TgBot::InputFile::fromFile("crossword.rtf", "file/rtf"));
    bot.getApi().sendDocument(id, TgBot::InputFile::fromFile("crossword.html", "file/html"));

    user.blocked = false;

    user.words.clear();
}

int callback(void* data, int argc, char** argv, char** azColName)
{
    users_count = std::stoi(argv[0]);
    return 0;
}


int main(int argc, char* argv[])
{
    std::ifstream file("token.txt");
    std::string token;
    getline(file, token);
    file.close();

    std::unordered_map<int64_t, User> users;

    TgBot::Bot bot(token);
    TgBot::InlineKeyboardMarkup::Ptr keyboard_lang(new TgBot::InlineKeyboardMarkup);
    MakeTwoButtonsKeyboard(keyboard_lang, u8"RU 🇷🇺", u8"EN 🇬🇧");
    TgBot::InlineKeyboardMarkup::Ptr keyboard_gen(new TgBot::InlineKeyboardMarkup);
    MakeTwoButtonsKeyboard(keyboard_gen, u8"YES ✅", u8"NO ❌");


    sqlite3* db;

    bot.getEvents().onAnyMessage([&bot, &users](TgBot::Message::Ptr message) {
        long long chatId = message->chat->id;

        // Проверяем, есть ли пользователь в карте
        if (users.find(chatId) == users.end()) {
            // Если пользователя нет, создаем нового и добавляем его в карту
            User newUser;
            users[chatId] = newUser;
        }
        });


    bot.getEvents().onUnknownCommand([&bot, &users](TgBot::Message::Ptr message)
        {
            User& user = users[message->chat->id];

            if (user.blocked) return;

            BadEnter(user);
            user.number_input_flag = false;

            if (user.language_flag)
                IncorrectMessages(bot, message, u8"Unknown Command.", user);
            else
                IncorrectMessages(bot, message, u8"Неизвестная команда.", user);
        });

    bot.getEvents().onCommand("instruction", [&bot, &users](TgBot::Message::Ptr message)
        {
            User& user = users[message->chat->id];

            if (user.blocked) return;
            BadEnter(user);
            user.number_input_flag = false;
            Instruction(bot, message, user);
        });

    bot.getEvents().onCommand("start", [&bot, &keyboard_lang, &users, &db](TgBot::Message::Ptr message)
        {
            User& user = users[message->chat->id];
            if (user.blocked) return;
            const std::string QUERY(
                "CREATE TABLE IF NOT EXISTS USERS("
                "USER_ID	INTEGER		NOT NULL,"
                "UNIQUE(USER_ID)"
                ");"
                "INSERT OR IGNORE INTO USERS(USER_ID) VALUES(" + std::to_string(message->from->id) + ");"
                "SELECT COUNT(*) from USERS; ");

            sqlite3_open("USERS.db", &db);
            sqlite3_exec(db, QUERY.c_str(), callback, 0, nullptr);
            sqlite3_close(db);

            BadEnter(user);
            user.number_input_flag = false;

            std::string message_text;
            if (user.language_flag)
                message_text = u8"Hi 👋 " + message->chat->firstName + u8"! Choose the language in which it is convenient for you to communicate with me.";
            else
                message_text = u8"Привет 👋 " + message->chat->firstName + u8"! Выберите язык на котором вам удобнее со мной общаться.";

            bot.getApi().sendMessage(message->chat->id, message_text, false, 0, keyboard_lang);
        });

    bot.getEvents().onCallbackQuery([&bot, &users](TgBot::CallbackQuery::Ptr query)
        {
            User& user = users[query->message->chat->id];
            if (user.blocked) return;
            user.number_input_flag = false;

            if (query->data == u8"NO ❌" || query->data == u8"YES ✅")
            {
                if (user.word_input_flag)
                {
                    if (user.language_flag)
                        bot.getApi().sendMessage(query->message->chat->id, u8"You didn't enter all the words. ✉️");
                    else
                        bot.getApi().sendMessage(query->message->chat->id, u8"Вы ввели не все слова. ✉️");
                }
                else if (query->data == u8"NO ❌")
                {
                    BadEnter(user);
                    if (user.language_flag)
                        IncorrectMessages(bot, query->message, u8"Words dropped.", user);
                    else
                        IncorrectMessages(bot, query->message, u8"Слова сброшены.", user);
                }
                else if (user.words.size())
                {
                    if (user.language_flag)
                        bot.getApi().sendMessage(query->message->chat->id, u8"Words accepted! Expect a crossword puzzle! This may take up to 5 minutes. ⏰");
                    else
                        bot.getApi().sendMessage(query->message->chat->id, u8"Слова приняты! Начинаю состовлять кроссворд! Это может занять до 5 минут. ⏰");

                    std::thread Thread(GenerateCrossword, std::ref(user.words), std::ref(bot), query->message->chat->id, std::ref(user));
                    Thread.detach();
                }
                return;
            }

            BadEnter(user);

            if (query->data == u8"RU 🇷🇺")
            {
                bot.getApi().sendMessage(query->message->chat->id, u8"Выбран русский 🇷🇺");
                user.language_flag = false;
                Instruction(bot, query->message, user);
            }
            else if (query->data == u8"EN 🇬🇧")
            {
                bot.getApi().sendMessage(query->message->chat->id, u8"Selected English 🇬🇧");
                user.language_flag = true;
                Instruction(bot, query->message, user);
            }
        });

    bot.getEvents().onCommand("generate", [&bot, &users](TgBot::Message::Ptr message)
        {
            User& user = users[message->chat->id];
            if (user.blocked) return;
            BadEnter(user);
            if (user.language_flag)
                bot.getApi().sendMessage(message->chat->id, u8"Enter number of words (2 - 20):");
            else
                bot.getApi().sendMessage(message->chat->id, u8"Введите число слов (2 - 20):");
            user.number_input_flag = true;
        });

    bot.getEvents().onNonCommandMessage([&bot, &users, &keyboard_gen](TgBot::Message::Ptr message)
        {

            User& user = users[message->chat->id];
            if (user.blocked) return;
            if (user.word_input_flag)
            {
                if (!CheckWord(message, user))
                {
                    if (user.language_flag)
                        bot.getApi().sendMessage(message->chat->id, u8"❌ The word does not match the requirements, try to enter a new one:");
                    else
                        bot.getApi().sendMessage(message->chat->id, u8"❌ Слово не соответствует требованиям, попробуйте ввести новое:");
                    return;
                }

                user.words.push_back(message->text);
                user.word_num--;

                if (!user.word_num)
                {
                    user.word_input_flag = false;

                    for (const auto& word : user.words)
                        user.entered_words += word + '\n';

                    if (user.language_flag)
                        bot.getApi().sendMessage(message->chat->id, user.entered_words + u8"Are all words entered correctly?", false, 0, keyboard_gen);
                    else
                        bot.getApi().sendMessage(message->chat->id, user.entered_words + u8"Все слова введены правильно?", false, 0, keyboard_gen);
                    user.entered_words.clear();
                }
                return;
            }
            try {
                if (user.number_input_flag)
                {
                    user.word_num = std::stoi(message->text);

                    if (user.word_num < 1 || user.word_num > 20)
                        throw std::invalid_argument(message->text);

                    if (user.language_flag)
                        bot.getApi().sendMessage(message->chat->id, u8"⚡ Enter " + message->text + u8" words from which I will generate a crossword puzzle for you:");
                    else
                        bot.getApi().sendMessage(message->chat->id, u8"⚡ Введите " + message->text + u8" слов, из которых я сгенерирую кроссворд для вас:");

                    user.number_input_flag = false;
                    user.word_input_flag = true;
                    return;
                }
                else
                    throw std::invalid_argument(message->text);
            }
            catch (std::invalid_argument&)
            {
                user.word_num = 0;
                user.number_input_flag = false;

                if (user.language_flag)
                    IncorrectMessages(bot, message, u8"Incorrest message.", user);
                else
                    IncorrectMessages(bot, message, u8"Неверное сообщение.", user);
                return;
            }
        });

    bot.getEvents().onCommand("users_count", [&bot](TgBot::Message::Ptr message)
        {
            if (users_count == -1)
                return;
            bot.getApi().sendMessage(message->chat->id, std::to_string(users_count));
        });

    try
    {
        printf("Bot username: %s\n", bot.getApi().getMe()->username.c_str());
        TgBot::TgLongPoll longPoll(bot);
        while (true)
        {
            try
            {
                printf("Long poll started\n");
                longPoll.start();
            }
            catch (TgBot::TgException& e)
            {
                if ( e.what() == "Forbidden: bot was blocked by the user" )
                {
                    for (auto& up : bot.getApi().getUpdates())
                        up = nullptr;
                }
                else
                    throw;
            }
        }
    }
    catch (TgBot::TgException& e)
    {
        printf("error: %s\n", e.what());
    }
    return 0;
}