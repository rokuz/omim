#pragma once

#include "geometry/point2d.hpp"

#include "platform/http_request.hpp"

#include "std/chrono.hpp"
#include "std/function.hpp"
#include "std/initializer_list.hpp"
#include "std/string.hpp"
#include "std/unique_ptr.hpp"
#include "std/utility.hpp"

class BookingApi
{
  string m_affiliateId;
  string m_apiUrl;

public:
  static constexpr const char kDefaultCurrency[1] = {0};

  struct Details
  {
    string m_bookingId;
    m2::PointD m_point;
    system_clock::time_point m_arrivalDate;
    system_clock::time_point m_departureDate;

    Details(string const & bookingId, m2::PointD const & point,
            system_clock::time_point const & arrivalDate,
            system_clock::time_point const & departureDate)
      : m_bookingId(bookingId)
      , m_point(point)
      , m_arrivalDate(arrivalDate)
      , m_departureDate(departureDate)
    {}
  };

  BookingApi();
  string GetBookingUrl(string const & baseUrl, string const & lang = string()) const;
  string GetDescriptionUrl(string const & baseUrl, string const & lang = string()) const;
  void GetMinPrice(string const & hotelId, string const & currency,
                   function<void(string const &, string const &)> const & fn);
  void GetBookingDetails(function<void(vector<Details> details)> const & fn);

protected:
  unique_ptr<downloader::HttpRequest> m_request;
  string MakeApiUrl(string const & func, initializer_list<pair<string, string>> const & params);
};
