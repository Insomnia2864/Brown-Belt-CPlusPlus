#include "Common.h"

#include <stdexcept>
#include <algorithm>

using namespace std;

class Figure : public IShape
{
public:
	void SetPosition(Point position) override
	{
		position_ = position;
	}

	Point GetPosition() const override
	{
		return position_;
	}

	void SetSize(Size size) override
	{
		size_ = size;
	}

	Size GetSize() const override
	{
		return size_;
	}

	void SetTexture(std::shared_ptr<ITexture> texture) override
	{
		if (texture_.get() != nullptr)
		{
			texture_.~shared_ptr();
		}
		texture_ = texture;
	}

	ITexture* GetTexture() const override
	{
		return texture_.get();
	}

	void Draw(Image& image) const override
	{
		if (image.empty())
		{
			return;
		}

		const Image& img = (texture_.get() == nullptr) ? Image(size_.height, string(size_.width, '.')) : texture_->GetImage();

		for (int i = position_.y; 
			i < min(static_cast<int>(image.size()), position_.y + size_.height);
			i++)
		{
			for (int j = position_.x; 
				j < min(static_cast<int>(image[i-position_.y].size()), position_.x + size_.width); 
				j++)
			{
				if (PointInFigure({ j - position_.x, i - position_.y }))
				{
					if (((i - position_.y) < static_cast<int>(img.size())) && ((j - position_.x) < static_cast<int>(img[i - position_.y].size())))
					{
						image[i][j] = img[i - position_.y][j - position_.x];
					}
					else
					{
						image[i][j] = '.';
					}
				}
			}
		}
	}

private:
	Point position_;
	Size size_;
	shared_ptr<ITexture> texture_;

	virtual bool PointInFigure(Point point) const = 0;

};


class Rectangle : public Figure
{
public:
	unique_ptr<IShape> Clone() const override
	{
		return make_unique<Rectangle>(*this);
	}
private:

	bool PointInFigure(Point point) const override
	{
		return true;
	}
};

class Ellipse : public Figure
{
public:
	unique_ptr<IShape> Clone() const override
	{
		return make_unique<Ellipse>(*this);
	}
private:
	bool PointInFigure(Point point) const override
	{
		return IsPointInEllipse(point, this->GetSize());
	}
};

unique_ptr<IShape> MakeShape(ShapeType shape_type) 
{
	if (shape_type == ShapeType::Rectangle)
	{
		return make_unique<Rectangle>();
	}
	else if (shape_type == ShapeType::Ellipse)
	{
		return make_unique<Ellipse>();
	}

	throw invalid_argument("MakeShape invalid argument");
}