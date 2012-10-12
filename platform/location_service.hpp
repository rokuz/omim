#pragma once

#include "location.hpp"

namespace location
{

class LocationObserver
{
public:
  virtual void OnLocationError(TLocationError errorCode) = 0;
  virtual void OnLocationUpdated(GpsInfo const & info) = 0;
};

class LocationService
{
protected:
  LocationObserver & m_observer;

public:
  LocationService(LocationObserver & observer) : m_observer(observer) {}
  virtual ~LocationService() {}

  virtual void Start() = 0;
  virtual void Stop() = 0;
};

} // namespace location

extern "C" location::LocationService * CreateDesktopLocationService(location::LocationObserver & observer);
