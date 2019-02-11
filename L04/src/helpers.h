#include <vector>
#include <iostream>

using namespace std;

namespace helpers
    {
        vector<int> range(int lower, int increment, int upper)
            {
                vector<int> result;
                if (increment > 0)
                    {
                        for (int i = lower; i <= upper; i += increment)
                            {
                                result.push_back(i);
                            }
                    }
                else
                    {
                        for (int i = lower; i >= upper; i += increment)
                            {
                                result.push_back(i);
                            }
                    }
                return result;
            }
        vector<int> range(int lower, int upper)
            {
                return range(lower, 1, upper);
            }
        vector<int> range(int upper)
            {
                return range(1, 1, upper);
            }
    };

