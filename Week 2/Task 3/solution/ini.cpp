#include "ini.h"

#include <iostream>
#include <algorithm>

namespace Ini
{
	Section& Document::AddSection(std::string name)
	{
		return sections[name];
	}

	const Section& Document::GetSection(const std::string& name) const
	{
		return sections.at(name);
	}

	std::size_t Document::SectionCount() const
	{
		return sections.size();
	}

	Document Load(std::istream& in)
	{
		Document doc;
		
		Section* section = nullptr;

		for (std::string line; std::getline(in, line);)
		{
			if (!line.empty())
			{
				if (line[0] == '[')
				{
					auto it = std::next(line.begin());
					auto it2 = std::find(it, line.end(), ']');
					std::string sec = std::string(it, it2);
					section = &doc.AddSection(sec);
				}
				else
				{

					auto iterat = std::find(line.begin(), line.end(), '=');
					std::string key(line.begin(), iterat);
					std::string value(std::next(iterat), line.end());

					(*section)[key] = value;
				}
			}
		}

		return doc;
	}
}