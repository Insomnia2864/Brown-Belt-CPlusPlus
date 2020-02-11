#pragma once

namespace RAII
{
	template <typename Provider>
	class Booking
	{
	public:
		Booking(Provider* provider, int id) :
			provider_(provider),
			id_(id)
		{}

		Booking(const Booking&) = delete;
		Booking& operator = (const Booking&) = delete;

		Booking(Booking&& other) 
		{
			provider_ = other.provider_;
			other.provider_ = nullptr;
			id_ = other.id_;
		}

		Booking& operator = (Booking&& other)
		{
			provider_ = other.provider_;
			other.provider_ = nullptr;
			id_ = other.id_;

			return *this;
		}

		~Booking()
		{
			if (provider_ != nullptr)
			{
				provider_->CancelOrComplete(*this);
			}
		}
	private:
		Provider* provider_; 
		int id_;
	};
}
