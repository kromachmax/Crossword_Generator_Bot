#include <iostream>
#include <vector>
#include <string>
#include<algorithm>
#include "Header.h"

Board::Board(std::vector<std::u16string>& w) : words(std::move(w)), used(words.size(), false) {

	std::sort(words.begin(), words.end(), [](const std::u16string& s1, const std::u16string& s2) {
		if (s1.size() > s2.size())
			return true;
		else if (s1.size() < s2.size())
			return false;
		return s1 < s2;
		});
}

std::u16string Board::Print_Board() {

	if (!CREATION)
		return u"ERROR";

	std::u16string crossword;

	for (size_t row = 0; row < board.size(); ++row) {
		for (size_t column = 0; column < board.size(); ++column) {
			if (board[row][column] != u'_') {
				Left_Bound = std::min(Left_Bound, column);
				Right_Bound = std::max(Right_Bound, column);
				Upper_Bound = std::min(Upper_Bound, row);
				Lower_Bound = std::max(Lower_Bound, row);
			}
		}
	}

	crossword += u"<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\"\n";
	crossword += u"\"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">\n";
	crossword += u"<html xmlns = \"http://www.w3.org/1999/xhtml\">\n";
	crossword += u"<head>\n";
	crossword += u"<meta http - equiv = \"Content-Type\" content = \"text/html; charset=utf-8\" / >\n";
	crossword += u"<style type = \"text/css\">\n";
	crossword += u"TABLE{";
	crossword += u"border: 4px double #399;\n";
	crossword += u"}\n";
	crossword += u"TD{\n";
	crossword += u"font-family:\"Courier New\";\n";
	crossword += u"background: #fc0;\n";
	crossword += u"border: 1px solid #333;\n";
	crossword += u"empty - cells: hide;\n";
	crossword += u"padding: 5px;\n";
	crossword += u"}\n";
	crossword += u"</style>\n";
	crossword += u"</head>\n";
	crossword += u"<body>\n";
	crossword += u"<table>\n";

	for (size_t row = Upper_Bound; row <= Lower_Bound; row++) {
		crossword += u"<tr>\n";
		for (size_t column = Left_Bound; column <= Right_Bound; ++column) {
			if (board[row][column] != u'_') {
				crossword += u"<td>";
				crossword += board[row][column];
				crossword += u"</td>";
			}
			else {
				crossword += u"<td style=\"background : white\">";
				crossword += u"</td>";
			}
		}
		crossword += u"\n</tr>\n";
	}

	crossword +=
		u"</table>\n"
		u"</body>\n"
		u"</html>\n";

	return crossword;
}



inline void Board::setSpot(char16_t c, int row, int col) noexcept {
	board[row][col] = c;
}

void Board::Generater(int count, direction direct) {

	if (clock() - start > delay) return;

	if (count == static_cast<int>(words.size())) {
		CREATION = true;
		return;
	}

	if (!count) {
		for (int j = 0; j < static_cast<int>(words[0].size()); j++)
			setSpot((words[0])[j], board.size() / 2, (board.size() - words[0].size()) / 2 + j);
		used[count] = true;
		Generater(count + 1, direction::DOWN);
		return;
	}


	for (int number_of_word = 1; number_of_word < static_cast<int>(words.size()); number_of_word++) {

		if (used[number_of_word])continue;

		if (direct == direction::ACROSS) {

			std::vector<good_word> Possible_options;
			if (placeNextHor(words[number_of_word], Possible_options))
				Horizontal_Proccessing(Possible_options, number_of_word, count);

			else if (placeNextVer(words[number_of_word], Possible_options))
				Vertical_Proccessing(Possible_options, number_of_word, count);

		}
		else {

			std::vector<good_word> Possible_options;
			if (placeNextVer(words[number_of_word], Possible_options))
				Vertical_Proccessing(Possible_options, number_of_word, count);

			else if (placeNextHor(words[number_of_word], Possible_options))
				Horizontal_Proccessing(Possible_options, number_of_word, count);

		}
	}
}

void Board::Vertical_Proccessing(std::vector<good_word>& Possible_options, int number_of_word, int count) {

	used[number_of_word] = true;

	std::sort(Possible_options.begin(), Possible_options.end(), [](const good_word& w1, const good_word& w2) {
		return w1.number_of_intersections > w2.number_of_intersections;
		});

	for (int i = 0; i < static_cast<int>(Possible_options.size()); i++) {

		Vertical_Placement(Possible_options[i], number_of_word);

		Generater(count + 1, direction::ACROSS);

		if (CREATION) return;

		Vertical_Cleaning(Possible_options[i], number_of_word);
	}
	used[number_of_word] = false;
}

void Board::Horizontal_Proccessing(std::vector<good_word>& Possible_options, int number_of_word, int count) {

	used[number_of_word] = true;

	std::sort(Possible_options.begin(), Possible_options.end(), [](const good_word& w1, const good_word& w2) {
		return w1.number_of_intersections > w2.number_of_intersections;
		});

	for (int i = 0; i < static_cast<int>(Possible_options.size()); i++) {

		Horizontal_Placement(Possible_options[i], number_of_word);

		Generater(count + 1, direction::DOWN);

		if (CREATION) return;

		Horizontal_Cleaning(Possible_options[i], number_of_word);
	}
	used[number_of_word] = false;
}


void Board::Vertical_Placement(const good_word& word, int number_of_word)noexcept {
	for (int i = 0; i < static_cast<int>(words[number_of_word].size()); i++) {
		if (board[word.row + i][word.colomn] != u'_') {
			check_intersection[word.row + i][word.colomn] = true;
			continue;
		}
		else board[word.row + i][word.colomn] = words[number_of_word][i];
	}
}

void Board::Horizontal_Placement(const good_word& word, int number_of_word)noexcept {
	for (int i = 0; i < static_cast<int>(words[number_of_word].size()); i++) {
		if (board[word.row][word.colomn + i] != u'_') {
			check_intersection[word.row][word.colomn + i] = true;
			continue;
		}
		else board[word.row][word.colomn + i] = words[number_of_word][i];
	}
}

void Board::Horizontal_Cleaning(const good_word& word, int number_of_word)noexcept {
	for (int i = 0; i < static_cast<int>(words[number_of_word].size()); i++) {
		if (check_intersection[word.row][word.colomn + i]) {
			check_intersection[word.row][word.colomn + i] = false;
			continue;
		}
		else board[word.row][word.colomn + i] = u'_';
	}
}

void Board::Vertical_Cleaning(const good_word& word, int number_of_word)noexcept {
	for (int i = 0; i < static_cast<int>(words[number_of_word].size()); i++) {
		if (check_intersection[word.row + i][word.colomn]) {
			check_intersection[word.row + i][word.colomn] = false;
			continue;
		}
		else board[word.row + i][word.colomn] = u'_';
	}
}

inline char16_t Board::getSpot(int i, int j)const noexcept {
	return board[i][j];
}

bool Board::placeNextVer(const std::u16string& word, std::vector<good_word>& options)const {
	int match = 0;
	bool key = 0;
	for (int j = 1; j < static_cast<int>(board.size()) - 1; j++) {
		for (int i = 1; i < static_cast<int>(board.size()) - static_cast<int>(word.size()) - 1; i++) {
			for (int k = 0; k < static_cast<int>(word.size()); k++) {

				if (getSpot(i + k, j) == u'_')
					match++;

				if (getSpot(i + k, j) == word[k]) {
					match++;
					if (getSpot(i + k + 1, j) == u'_' && getSpot(i + k - 1, j) == u'_') {
						match += 2;
						key++;
					}
					else break;
				}

				if (getSpot(i + k, j + 1) == u'_' && getSpot(i + k, j - 1) == u'_')
					match += 2;

				if (getSpot(i - 1, j) == u'_' && getSpot((i + word.size()), j) == u'_') {
					if (k == 0) match += 2;
				}
				else break;

			}
			if (match == 3 * word.size() + 2 && key)
				options.push_back(good_word{ size_t(i), size_t(j), size_t(key) });
			match = 0;
			key = 0;
		}
	}
	return options.size();
}

bool Board::placeNextHor(const std::u16string& word, std::vector<good_word>& options)const {
	int match = 0;
	int key = 0;
	for (int j = 1; j < static_cast<int>(board.size()) - 1; j++) {
		for (int i = 1; i < static_cast<int>(board.size()) - static_cast<int>(word.size()) - 1; i++) {
			for (int k = 0; k < static_cast<int>(word.size()); k++) {

				if (getSpot(j, i + k) == u'_')
					match++;

				if (getSpot(j, i + k) == word[k]) {
					match++;
					if (getSpot(j, i + k + 1) == '_' && getSpot(j, i + k - 1) == '_') {
						match += 2;
						key++;
					}
					else break;
				}

				if (getSpot(j + 1, i + k) == '_' && getSpot(j - 1, i + k) == '_')
					match += 2;

				if (getSpot(j, i - 1) == '_' && getSpot(j, i + word.size()) == '_') {
					if (k == 0) match += 2;
				}
				else break;

			}

			if (match == 3 * word.size() + 2 && key)
				options.push_back(good_word{ size_t(j), size_t(i), size_t(key) });
			match = 0;
			key = 0;
		}
	}
	return options.size();
}