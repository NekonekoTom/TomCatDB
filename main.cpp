/**
 * @file main.cpp
 * @author TomCat (noemail@noemail.com)
 * @brief Main test
 * @version 0.1
 * @date 2022-09-23
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <ctime>
#include "comparator.h"
#include "csv.h"
#include "db.h"
#include "db_table.h"
#include "mem_allocator.h"
#include "reader.h"
#include "sequence.h"
#include "skiplist.h"
#include "thread_pool.h"
#include "tools.h"
#include "varint.h"

int main() {
  Config config;
  TCDB db(config);

  // TCThreadPool thread_pool(4, 10);
  // thread_pool.Start();

  // auto job = [](int x, int y) {
  //   std::cout << "Computing...\n";
  //   std::this_thread::sleep_for(std::chrono::milliseconds(200));
  //   std::cout << "Done...\n";
  // };

  // thread_pool.SubmitTask(job, 1, 2);
  // thread_pool.SubmitTask(job, 1, 2);
  // thread_pool.SubmitTask(job, 1, 2);
  // thread_pool.SubmitTask(job, 1, 2);
  // thread_pool.SubmitTask(job, 1, 2);
  // thread_pool.SubmitTask(job, 1, 2);
  // thread_pool.SubmitTask(job, 1, 2);
  // thread_pool.SubmitTask(job, 1, 2);
  // thread_pool.SubmitTask(job, 1, 2);

  // Status ret(Status::NoError());
  int records = 20000;
  auto csv_data = CSVParser::ReadCSV(
      "/home/tom_cat/workdir/private/CS/C++/Primer/TomCatDB/test_data/test.csv",
      records);

  // Big data test
  for (int circle = 0; circle < 200; ++circle) {
    for (int i = 0; i < records; ++i) {
      db.Insert(csv_data[i][0], csv_data[i][10]); // id and description
      // db.Insert(csv_data[i][0], csv_data[i][20]);  // id and name
    }
  }

  // // Small data test
  // for (int i = 0; i < records; ++i) {
  //   db.Insert(csv_data[i][0], csv_data[i][10]); // id and description
  //   // db.Insert(csv_data[i][0], csv_data[i][20]);  // id and name
  // }

  auto entry_set = db.EntrySet();

  // auto val = db.Get(std::string("14402792"));
  auto val1 = db.Get(std::string("id"));
  auto val2 = db.Get(std::string("3895911"));
  auto val3 = db.Get(std::string("13911206"));

  db.TestEntryPoint();
  return 0;
}
/*
3895911,
Apartment,
Private room,
"{TV,""Cable TV"",Kitchen,""Free parking on premises"",Breakfast,""Elevator in building"",Heating,Washer,Dryer,""Smoke detector"",""First aid kit"",""Safety card"",""Fire extinguisher"",Essentials,Shampoo,Hangers,""Hair dryer"",Iron,""Laptop friendly workspace"",""translation missing: en.hosting_amenity_49"",""translation missing: en.hosting_amenity_50""}",
2,
1.0,
Real Bed,
flexible,
True,
LA,
"Close to SM beaches, 3rd Street Promenade, SM Pier & Venice, Montana Avenue, St. John's and SM Hospitals. You’ll love my place because of it's perfect location in the heart of Santa Monica. It's walking distance to great restaurants, coffee shops, bus lines, beaches, bike rentals, shopping, markets, drug stores, theaters, and more! You will have your own clean and private separate wing with a private bath and shower.  It's great for couples, solo adventurer, and business travelers. Guests will have access to a quaint clean, comfortable private bedroom/bathroom in a separate wing of the apartment, off the front entry. No other occupants will be in the private wing. You will have a view of a shared courtyard from your room's balcony. I am very flexible.  I enjoy meeting new people from different places, and I respect your privacy.  Please feel free to ask about directions or local places that you may want to know about.  I will do my best to assist you to make your stay as enjoyable as p",
2016-10-23,
t,
f,
100%,
2016-08-13,
f,
2017-02-26,
34.028372378220894,
-118.49444940110756,
Santa Monica Private Bedroom/Bathroom Suite,
Santa Monica,
6,
97.0,
https://a0.muscache.com/im/pictures/92355eae-b660-437d-b91a-2c344015751c.jpg?aki_policy=small,
90403,
1.0,
1.0

9710289,Apartment,Entire home/apt,"{TV,""Cable TV"",""Wireless Internet"",""Air conditioning"",Kitchen,Gym,Elevator,Heating,Washer,Dryer,""Smoke detector"",""Carbon monoxide detector"",Essentials,""Hair dryer"",""Laptop friendly workspace"",""translation missing: en.hosting_amenity_49"",""translation missing: en.hosting_amenity_50"",""Self Check-In"",Lockbox,""Hot water"",""Bed linens""}",3,1.0,Real Bed,moderate,True,NYC,"This apartment will give you a true (luxury) Brooklyn living experience. You'll stay in a newly-renovated historic catholic schoolhouse with huge windows, designer details and an incredible rooftop view. You'll be a few minutes walk from McCarren Park, McGolrick park, tons of amazing restaurants and bars, and the L train and G train. Steps away from both charming Greenpoint and hip Williamsburg. Everything you need is within walking distance. Mostly a very quiet, relaxed neighborhood. The space is bright with large windows, lots of plants, and farmhouse chic designer details. Enjoy full cable and chromecast, a large bathroom, and a fully stocked kitchen. You'll have access to the rooftop with the gorgeous views of Manhattan (the best view in Brooklyn!), a rec room with a pool table and TV, gym, and laundry room. I'll always be available by phone or email. Text me if you need me. This hip Brooklyn neighborhood (East Williamsburg, on the boarder of Greenpoint) is growing so fast. There a",2016-09-12,t,t,100%,2013-12-04,f,2016-10-16,40.720380083292326,-73.94232924998272,"Bright, charming luxury 1 BR with amazing rooftop",Williamsburg,2,80.0,https://a0.muscache.com/im/pictures/da03e413-d7d4-4a25-9aeb-9b7734831c11.jpg?aki_policy=small,11222,1.0,1.0
*/