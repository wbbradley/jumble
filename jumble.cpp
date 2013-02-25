// REVIEWED
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include "cmd_options.h"
#include "disk.h"
#include "utils.h"

const char *option_jumble = "jumble";
const char *option_verbose = "verbose";

cmd_option_t cmd_options[] =
{
	{ option_jumble, "-j" /*opt*/, true /*mandatory*/, true /*has_data*/ },
	{ option_verbose, "-v" /*opt*/, false /*mandatory*/, false /*has_data*/ },
};

std::vector<std::string> dictionary;

void load_dict()
{
	FILE *fp = fopen("dictionary.txt", "rt");
	if (fp != NULL)
	{
		char buffer[2048];
		while (!feof(fp))
		{
			buffer[0] = 0;
			fgets(buffer, sizeof(buffer), fp);
			std::string str(buffer);
			if (str.size() != 0)
			{
				if (str[str.size() - 1] == '\n')
					str = std::string(str.c_str(), str.size() - 1);
				if (str[str.size() - 1] == '\r')
					str = std::string(str.c_str(), str.size() - 1);
				for (auto &ch : str)
					ch = tolower(ch);
				dictionary.push_back(str);
			}
		}
		fclose(fp);
	}
}

int main(int argc, char *argv[])
{
	load_dict();

	cmd_options_t options;
	if (!get_options(argc, argv, cmd_options, countof(cmd_options), options))
		return EXIT_FAILURE;

	if (get_option_exists(options, option_verbose))
		log_enable(log_error | log_warning | log_info);
	else
		log_enable(log_error);

	std::string jumble;
	get_option(options, option_jumble, jumble);

	//dlog(log_info, "looking for jumbles of %s\n",
			//jumble.c_str());

	/* find all subsets of the jumbled string of varying sizes */
	unordered_set<std::string> subsets;
	for (int i = jumble.size() - 1; i >= 0; --i)
	{
		std::string substring(jumble.c_str() + i, 1);
		//dlog(log_info, "substring is %s\n", substring.c_str());

		unordered_set<std::string> potential_subsets;	
		for (auto &str : subsets)
		{
			//dlog(log_info, "adding %s + %s\n", substring.c_str(), str.c_str());
			std::string choice = substring + str;
			potential_subsets.insert(choice);
		}
		subsets.insert(substring);
		for (auto &str : potential_subsets)
			subsets.insert(str);
	}

	unordered_map<std::string, std::vector<std::string>> map;
	for (auto &str : dictionary)
	{
		//printf("%s\n", str.c_str());
		std::string sorted(str);
		std::sort(sorted.begin(), sorted.end());
//		if (str.size() <= 8)
//			printf("adding %s to %s\n", str.c_str(), sorted.c_str());
		
		map[sorted].push_back(str);
	}

	unordered_set<std::string> words_found;
	for (auto &str : subsets)
	{
		std::string sorted(str);
		std::sort(sorted.begin(), sorted.end());
		auto words_iter = map.find(sorted);
//		printf("looking for %s sorted (%s)\n", str.c_str(), sorted.c_str());
		if (words_iter != map.end())
		{
			for (auto &word : words_iter->second)
			{
				if (words_found.find(word) == words_found.end())
				{
					words_found.insert(word);
					printf("%s\n", word.c_str());
				}
			}
		}
	}

	printf("found %d matches\n", (int)words_found.size());
	return 0;
}

