#ifndef __SENSOR_DATA_H__
#define __SENSOR_DATA_H__

#include <ctime>
#include <string>

class SensorData
{
  protected:
    time_t timestamp;
    double avg;
    string unit;

  public:
    SensorData(time_t timestamp, double avg, string unit) 
    {
        this->timestamp = timestamp;
        this->avg = avg;
        this->unit = unit;
    }
    virtual ~SensorData() {}

    void setValue(double value) { this->avg = value; }
    double getValue() { return this->avg; }

    void setTimestamp(time_t timestamp) { this->timestamp = timestamp; }
    time_t getTimestamp() { return this->timestamp; }

    string getUnit() { return this->unit; }
};

#endif /* __SENSOR_DATA_H__ */