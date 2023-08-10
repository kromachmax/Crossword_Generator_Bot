#pragma once
#include <vector>
#include <string>
#include<ctime>
#include<uchar.h>
#include <codecvt>

class Board {
private:
	std::vector<std::u16string>words;
	std::vector < std::vector<char16_t>>board = std::vector<std::vector<char16_t>>(50, std::vector<char16_t>(50, u'_'));
	std::vector<std::vector<bool>> check_intersection = std::vector<std::vector<bool>>(50, std::vector<bool>(50, false));

	struct good_word {
		size_t row;
		size_t colomn;
		size_t number_of_intersections;
	};
	std::vector<bool> used;
	inline char16_t getSpot(int, int)const noexcept;
	inline void setSpot(char16_t, int, int)noexcept;
	bool CREATION = false;
	bool placeNextVer(const std::u16string&, std::vector<good_word>&)const;
	bool placeNextHor(const std::u16string&, std::vector<good_word>&)const;
	void Vertical_Placement(const good_word& word, int number_of_word)noexcept;
	void Horizontal_Placement(const good_word& word, int number_of_word)noexcept;
	void Horizontal_Cleaning(const good_word& word, int number_of_word)noexcept;
	void Vertical_Cleaning(const good_word& word, int number_of_word)noexcept;
	void Vertical_Proccessing(std::vector<good_word>& Possible_Options, int number_of_word, int count);
	void Horizontal_Proccessing(std::vector<good_word>& Possible_Options, int number_of_word, int count);
	size_t Upper_Bound = 50;
	size_t  Lower_Bound = 0;
	size_t Left_Bound = 50;
	size_t Right_Bound = 0;
	clock_t start = clock();
	clock_t delay = 300 * CLOCKS_PER_SEC;

public:
	enum class direction {
		ACROSS,
		DOWN
	};
	Board(std::vector<std::u16string>& w);
	Board() = default;
	~Board() = default;
	void Generater(int number_of_word, direction direct);
	std::u16string Print_Board();
};
