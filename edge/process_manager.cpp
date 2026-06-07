#include "process_manager.h"
#include "opcode.h"
#include "byte_op.h"
#include "setting.h"
#include <cstring>
#include <iostream>
#include <ctime>
#include <fstream>
#include <string>
using namespace std;

ProcessManager::ProcessManager()
{
  this->num = 0;
  this->vectorType = 1;
}

void ProcessManager::init()
{
  this->vectorType = this->loadVectorType();
  cout << "[*] Vector type: " << this->vectorType << endl;
}

int ProcessManager::loadVectorType()
{
  ifstream fin("vector_config.txt");
  string line;

  if (!fin.is_open())
    return 1;

  while (getline(fin, line))
  {
    for (size_t i=0; i<line.length(); i++)
    {
      if (line[i] >= '1' && line[i] <= '3')
        return line[i] - '0';
    }
  }

  return 1;
}

// TODO: You should implement this function if you want to change the result of the aggregation
uint8_t *ProcessManager::processData(DataSet *ds, int *dlen)
{
  uint8_t *ret, *p;
  int num;
  HouseData *house;
  TemperatureData *tdata;
  HumidityData *hdata;
  PowerData *pdata;
  ret = (uint8_t *)malloc(BUFLEN);
  int tmp, min_humid, min_temp, min_power, month;
  int avg_humid, avg_temp, avg_power;
  int range_humid, range_temp;
  int sum_power;
  time_t ts;
  struct tm *tm;

  tdata = ds->getTemperatureData();
  hdata = ds->getHumidityData();
  num = ds->getNumHouseData();

  min_temp = (int) tdata->getMin();
  min_humid = (int) hdata->getMin();
  avg_temp = (int) tdata->getValue();
  avg_humid = (int) hdata->getValue();
  range_temp = (int) (tdata->getMax() - tdata->getMin());
  range_humid = (int) (hdata->getMax() - hdata->getMin());

  min_power = 10000;
  sum_power = 0;
  for (int i=0; i<num; i++)
  {
    house = ds->getHouseData(i);
    pdata = house->getPowerData();
    tmp = (int)pdata->getValue();

    if (tmp < min_power)
      min_power = tmp;
    sum_power += tmp;
  }
  avg_power = sum_power / num;

  ts = ds->getTimestamp();
  tm = localtime(&ts);
  month = tm->tm_mon + 1;

  memset(ret, 0, BUFLEN);
  *dlen = 0;
  p = ret;

  if (this->vectorType == 1)
  {
    // Vector 1: minimum-based vector
    VAR_TO_MEM_1BYTE_BIG_ENDIAN(min_temp, p);
    *dlen += 1;
    VAR_TO_MEM_1BYTE_BIG_ENDIAN(min_humid, p);
    *dlen += 1;
    VAR_TO_MEM_2BYTES_BIG_ENDIAN(min_power, p);
    *dlen += 2;
  }
  else if (this->vectorType == 2)
  {
    // Vector 2: average-based vector
    VAR_TO_MEM_1BYTE_BIG_ENDIAN(avg_temp, p);
    *dlen += 1;
    VAR_TO_MEM_1BYTE_BIG_ENDIAN(avg_humid, p);
    *dlen += 1;
    VAR_TO_MEM_2BYTES_BIG_ENDIAN(avg_power, p);
    *dlen += 2;
  }
  else
  {
    // Vector 3: range-based weather vector with average power target
    VAR_TO_MEM_1BYTE_BIG_ENDIAN(range_temp, p);
    *dlen += 1;
    VAR_TO_MEM_1BYTE_BIG_ENDIAN(range_humid, p);
    *dlen += 1;
    VAR_TO_MEM_2BYTES_BIG_ENDIAN(avg_power, p);
    *dlen += 2;
  }

  VAR_TO_MEM_1BYTE_BIG_ENDIAN(month, p);
  *dlen += 1;

  VAR_TO_MEM_1BYTE_BIG_ENDIAN(this->vectorType, p);
  *dlen += 1;

  return ret;
}