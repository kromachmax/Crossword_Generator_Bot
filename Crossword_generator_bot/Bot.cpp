#include <stdio.h>
#include <tgbot/tgbot.h>
#include<fstream>
#include<uchar.h>
#include <codecvt>
#include"Header.h"
#include <Windows.h>
#include<ctime>
#include<utility>
#include<execution>
#include<sqlite3.h>

void IncorrectMessages(TgBot::Bot& bot, TgBot::Message::Ptr& message, std::string&& mes, bool& language_flag) {

    if (language_flag) bot.getApi().sendMessage(message->chat->id, mes +
        " Use /instruction for information on how to use the bot. \xF0\x9F\x94\xA7");
    else bot.getApi().sendMessage(message->chat->id, mes +
        " Используйте команду /instruction, для получения информации о боте. \xF0\x9F\x94\xA7");

}


int users_count = 0;

int callback(void* date, int argc, char** argv, char** azColName) {
    users_count = std::stoi(argv[0]);
    return 0;
}

void Instruction(TgBot::Bot& bot, TgBot::Message::Ptr& message, bool& language_flag) {

    if (language_flag) bot.getApi().sendMessage(message->chat->id,
        "\xF0\x9F\x94\xB8 To start, enter the /start command and select the language.\n"
        "\xF0\x9F\x94\xB8 To get instructions for use, enter /instruction. \xF0\x9F\x94\xA7\n"
        "\xF0\x9F\x94\xB8 To start generating the crossword, type /generate.\nThen you need to enter the number of "
        "words that the crossword will consist of. (2 - 30)\n"
        "Then enter the words, one at a time in the message.\nThe word length can be no more than 20 characters.\n"
        "A word can consist only of letters of the Russian or English alphabet.\n"
        "After making sure that the entered words are correct, click \"YES\" and expect the result.\n"
        "(up to 5 minutes) \xE2\x8F\xB0\n"
        "\xE2\xAD\x90 Have a good use! \xE2\xAD\x90");

    else bot.getApi().sendMessage(message->chat->id,
        "\xF0\x9F\x94\xB8 Используйте команду /start, для начала работы и выбора языка.\n"
        "\xF0\x9F\x94\xB8 Используйте команду /instruction, для получения информации о боте. \xF0\x9F\x94\xA7\n"
        "\xF0\x9F\x94\xB8 Для того чтобы приступить к генерации кроссворда, используйте команду /generate.\n"
        "Затем введите число слов, из которых будет состоять кроссворд. (2 - 30)\n"
        "Затем введите слова, по одному в сообщении.\nДлина слова не более 20 символов.\n"
        "Слово может состоять только из букв русского или английского алфавитов.\n"
        "После убедитесь в правильности введенной информации и нажмите \"YES\". Ожидайте результат."
        "\n(занимает до 5 минут) \xE2\x8F\xB0\n"
        "\xE2\xAD\x90 Приятного использования! \xE2\xAD\x90");

}

void MakeTwoButtonsKeyboard(TgBot::InlineKeyboardMarkup::Ptr& keyboard, const std::string& b1, const std::string& b2) {

    std::vector<TgBot::InlineKeyboardButton::Ptr> buttons{ std::make_shared<TgBot::InlineKeyboardButton>(),
        std::make_shared<TgBot::InlineKeyboardButton>() };
    buttons[0]->text = b1;
    buttons[0]->callbackData = b1;
    buttons[1]->text = b2;
    buttons[1]->callbackData = b2;
    keyboard->inlineKeyboard.push_back(buttons);

}

bool toupper_u16(char16_t& ch) {
    if (ch >= 1040 and ch <= 1071) return true;
    else if (ch >= 1072 and ch <= 1103) {
        ch -= 32;
        return true;
    }
    else if (ch == 1105) {
        ch = 1025;
        return true;
    }
    else if (ch == 1025) return true;
    else return false;
}


bool CheckWord(TgBot::Message::Ptr& message, bool language_flag, std::u16string& str) {


    std::wstring_convert<std::codecvt<char16_t, char, std::mbstate_t>, char16_t> convert;
    str = convert.from_bytes(message->text);


    if (str.size() > 20 || !str.size() || message->text.find(u' ') != std::string::npos)
        return false;

    if (language_flag) {

        for (auto& sim : str) {
            if (!((sim = std::toupper(sim)) >= 65 && (sim = std::toupper(sim)) <= 90))
                return false;
        }
        return true;

    }
    else {

        for (auto& sim : str) {
            if (!toupper_u16(sim))
                return false;
        }
        return true;
    }
}

void BadEnter(std::vector<std::u16string>& words, int& word_num, bool& word_input_flag) {

    words.clear();
    word_num = 0;
    word_input_flag = false;

}

int main(int argc, char* argv[]) {

    SetConsoleOutputCP(65001);

    TgBot::Bot bot("6401871694:AAF8NbDy684KxdD8yip28D9E6LKQ5k0pnIg");

    sqlite3* db;

    TgBot::Message::Ptr message;

    TgBot::InlineKeyboardMarkup::Ptr keyboard_lang(new TgBot::InlineKeyboardMarkup);
    MakeTwoButtonsKeyboard(keyboard_lang, "RU \xF0\x9F\x87\xB7\xF0\x9F\x87\xBA", "EN \xF0\x9F\x87\xAC\xF0\x9F\x87\xA7");

    TgBot::InlineKeyboardMarkup::Ptr keyboard_gen(new TgBot::InlineKeyboardMarkup);
    MakeTwoButtonsKeyboard(keyboard_gen, "YES \xE2\x9C\x85", "NO \xE2\x9D\x8C");

    bool language_flag = true;
    bool number_input_flag = false;
    bool word_input_flag = false;
    bool block_flag = false;
    int word_num = 0;

    std::u16string entered_words;
    std::vector<std::u16string> words;

    bot.getEvents().onUnknownCommand([&bot, &number_input_flag, &word_input_flag, &words,
        &word_num, &block_flag, &language_flag](TgBot::Message::Ptr message) {

            if (block_flag)
                return;

            BadEnter(words, word_num, word_input_flag);
            number_input_flag = false;

            if (language_flag)
                IncorrectMessages(bot, message, "Unknown Command.", language_flag);
            else
                IncorrectMessages(bot, message, "Неизвестная команда.", language_flag);

        });

    bot.getEvents().onCommand("instruction", [&bot, &number_input_flag, &word_input_flag, &words,
        &word_num, &block_flag, &language_flag](TgBot::Message::Ptr message) {

            if (block_flag)
                return;

            BadEnter(words, word_num, word_input_flag);
            number_input_flag = false;
            Instruction(bot, message, language_flag);

        });

    bot.getEvents().onCommand("start", [&bot, &number_input_flag, &keyboard_lang, &word_input_flag,
        &words, &word_num, &block_flag, &language_flag, &db](TgBot::Message::Ptr message) {

            if (block_flag)
                return;

            const std::string QUERY(
                "CREATE TABLE IF NOT EXISTS USERS("
                "USER_ID  INTEGER  NOT NULL,"
                "UNIQUE(USER_ID)"
                ");"
                "INSERT OR IGNORE INTO USERS(USER_ID) VALUES(" + std::to_string(message->from->id) + ");"
                "SELECT COUNT(*) from USERS; ");

            sqlite3_open("USERS.db", &db);
            sqlite3_exec(db, QUERY.c_str(), callback, 0, nullptr);
            sqlite3_close(db);


            BadEnter(words, word_num, word_input_flag);
            number_input_flag = false;

            if (language_flag)
                bot.getApi().sendMessage(message->chat->id, "Hi \xF0\x9F\x91\x8B " + message->chat->firstName + "! Choose the "
                    "language in which it is convenient for you to communicate with me.", false, 0, keyboard_lang);
            else
                bot.getApi().sendMessage(message->chat->id, "Привет \xF0\x9F\x91\x8B " + message->chat->firstName + "! Выберите "
                    "язык на котором вам удобнее со мной общаться.", false, 0, keyboard_lang);

        });


    bot.getEvents().onCallbackQuery([&bot, &number_input_flag, &language_flag, &word_input_flag,
        &words, &word_num, &block_flag](TgBot::CallbackQuery::Ptr query) {


            std::wstring_convert<std::codecvt<char16_t, char, std::mbstate_t>, char16_t> convert;
            std::ofstream file("file.rtf");

            if (block_flag)
                return;

            number_input_flag = false;

            if (query->data == "NO \xE2\x9D\x8C" || query->data == "YES \xE2\x9C\x85") {
                if (word_input_flag) {
                    if (language_flag)
                        bot.getApi().sendMessage(query->message->chat->id, "You didn't enter all the words. \xE2\x9C\x89");
                    else
                        bot.getApi().sendMessage(query->message->chat->id, "Вы ввели не все слова. \xE2\x9C\x89");
                }
                else if (query->data == "NO \xE2\x9D\x8C") {
                    BadEnter(words, word_num, word_input_flag);
                    if (language_flag)
                        IncorrectMessages(bot, query->message, "Words dropped.", language_flag);
                    else
                        IncorrectMessages(bot, query->message, "Слова сброшены.", language_flag);
                }
                else if (words.size()) {
                    if (language_flag)
                        bot.getApi().sendMessage(query->message->chat->id,
                            "Words accepted! Expect a crossword puzzle! This may take up to 5 minutes. \xE2\x8F\xB0");
                    else
                        bot.getApi().sendMessage(query->message->chat->id,
                            "Слова приняты! Начинаю состовлять кроссворд! Это может занять до 5 минут. \xE2\x8F\xB0");

                    block_flag = true;
                    Board bot_generator(words);
                    bot_generator.Generater(0, Board::direction::ACROSS);
                    file << convert.to_bytes(bot_generator.Print_Board());
                    file.close();
                    bot.getApi().sendDocument(query->message->chat->id, TgBot::InputFile::fromFile("file.rtf", "file/rtf"));
                    words.clear();
                    block_flag = false;
                }
                return;
            }

            BadEnter(words, word_num, word_input_flag);

            if (query->data == "RU \xF0\x9F\x87\xB7\xF0\x9F\x87\xBA") {
                bot.getApi().sendMessage(query->message->chat->id, "Выбран русский \xF0\x9F\x87\xB7\xF0\x9F\x87\xBA");
                language_flag = false;
                Instruction(bot, query->message, language_flag);
            }
            else if (query->data == "EN \xF0\x9F\x87\xAC\xF0\x9F\x87\xA7") {
                bot.getApi().sendMessage(query->message->chat->id, "Selected English \xF0\x9F\x87\xAC\xF0\x9F\x87\xA7");
                language_flag = true;
                Instruction(bot, query->message, language_flag);
            }

        });

    bot.getEvents().onCommand("generate", [&bot, &number_input_flag, &words, &word_num,
        &word_input_flag, &block_flag, &language_flag](TgBot::Message::Ptr message) {

            if (block_flag)
                return;

            BadEnter(words, word_num, word_input_flag);

            if (language_flag)
                bot.getApi().sendMessage(message->chat->id, "Enter number of words (2 - 30):");
            else
                bot.getApi().sendMessage(message->chat->id, "Введите число слов (2 - 30):");

            number_input_flag = true;

        });

    bot.getEvents().onCommand("users_count", [&bot, &block_flag](TgBot::Message::Ptr message) {

        if (block_flag)
            return;

        if (users_count == 0)
            return;

        bot.getApi().sendMessage(message->chat->id, std::to_string(users_count));

        });

    bot.getEvents().onNonCommandMessage([&bot, &number_input_flag, &word_num, &word_input_flag,
        &words, &keyboard_gen, &entered_words, &block_flag, &language_flag](TgBot::Message::Ptr message) {

            std::wstring_convert<std::codecvt<char16_t, char, std::mbstate_t>, char16_t> convert;

            if (block_flag)
                return;

            if (word_input_flag) {

                std::u16string str = convert.from_bytes(message->text);

                if (!CheckWord(message, language_flag, str)) {

                    if (language_flag)
                        bot.getApi().sendMessage(message->chat->id,
                            "\xF0\x9F\x9A\xAB The word does not match the requirements, try to enter a new one:");
                    else
                        bot.getApi().sendMessage(message->chat->id,
                            "\xF0\x9F\x9A\xAB Слово не соответствует требованиям, попробуйте ввести новое:");
                    return;
                }

                words.push_back(str);
                --word_num;

                if (!word_num) {
                    word_input_flag = false;
                    for (const auto& word : words)
                        entered_words += word + u'\n';
                    if (language_flag)
                        bot.getApi().sendMessage(message->chat->id, convert.to_bytes(entered_words) +
                            "Are all words entered correctly?", false, 0, keyboard_gen);
                    else
                        bot.getApi().sendMessage(message->chat->id, convert.to_bytes(entered_words) +
                            "Все слова введены правильно?", false, 0, keyboard_gen);

                    entered_words.clear();
                }
                return;
            }
            try {
                if (number_input_flag) {
                    word_num = std::stoi(message->text);

                    if (word_num < 1 || word_num > 30)
                        throw std::invalid_argument(message->text);

                    if (language_flag)
                        bot.getApi().sendMessage(message->chat->id, "\xE2\x9A\xA1 Enter " + message->text +
                            " words from which I will generate a crossword puzzle for you:");
                    else
                        bot.getApi().sendMessage(message->chat->id, "\xE2\x9A\xA1 Введите " + message->text +
                            " слов, из которых я сгенерирую кроссворд для вас:");

                    number_input_flag = false;
                    word_input_flag = true;
                    return;
                }
                else
                    throw std::invalid_argument(message->text);
            }
            catch (std::invalid_argument&) {
                word_num = 0;
                number_input_flag = false;

                if (language_flag)
                    IncorrectMessages(bot, message, "Incorrest message.", language_flag);
                else
                    IncorrectMessages(bot, message, "Неверное сообщение.", language_flag);
                return;
            }

        });

    try {
        printf("Bot username: %s\n", bot.getApi().getMe()->username.c_str());
        TgBot::TgLongPoll longPoll(bot);
        while (true) {
            try {
                printf("Long poll started\n");
                longPoll.start();
            }
            catch (TgBot::TgException& e) {
                std::string one(e.what());
                std::string two("Forbidden: bot was blocked by the user");
                if (one == two) {
                    for (auto& i : bot.getApi().getUpdates()) {
                        i->message = nullptr;
                    }
                }
                else throw;
            }
        }
    }
    catch (TgBot::TgException& e) {
        printf("error: %s\n", e.what());
    }

    return 0;
}