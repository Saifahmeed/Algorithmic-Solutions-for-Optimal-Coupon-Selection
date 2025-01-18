#include <iostream>
#include <vector>
#include <algorithm>
#include <fstream>
#include <string>
#include <sstream>
#include <limits>
#include <tuple>

using namespace std;

class Coupon {
public:
    int id;
    double discountValue;
    int expiryDate;
    int costRequirement;

    Coupon(int id, double discount, int expiry, int costReq)
        : id(id), discountValue(discount), expiryDate(expiry), costRequirement(costReq) {
    }
    string toString() const {
        return to_string(id) + "," + to_string(discountValue) + "," + to_string(expiryDate) + "," + to_string(costRequirement);
    }
};

// Function to load coupons from a file
vector<Coupon> loadCoupons(const string& filename) {
    vector<Coupon> coupons;
    ifstream file(filename);
    string line;

    while (getline(file, line)) {
        stringstream ss(line);
        string token;
        int id, expiry, costReq;
        double discount;

        getline(ss, token, ',');
        id = stoi(token);

        getline(ss, token, ',');
        discount = stod(token);

        getline(ss, token, ',');
        expiry = stoi(token);

        getline(ss, token, ',');
        costReq = stoi(token);

        coupons.push_back(Coupon(id, discount, expiry, costReq));
    }

    file.close();
    return coupons;
}

// Function to save coupons to a file
void saveCoupons(const vector<Coupon>& coupons, const string& filename) {
    ofstream file(filename);
    for (const auto& coupon : coupons) {
        file << coupon.toString() << endl;
    }
    file.close();
}

// Function to remove expired coupons
void removeExpiredCoupons(vector<Coupon>& coupons, int today) {
    vector<Coupon> validCoupons;
    for (const auto& coupon : coupons) {
        if (coupon.expiryDate >= today) {
            validCoupons.push_back(coupon);
        }
    }
    coupons = validCoupons;  // Update the coupons list with valid ones
    cout << "Coupons that expired before today have been removed.\n";
}

// Function to display the coupons
void displayCoupons(const vector<Coupon>& coupons, const vector<Coupon>& selectedCoupons, int userCost) {
    double finalCost = userCost;
    for (const auto& coupon : selectedCoupons) {
        finalCost -= coupon.discountValue;
    }

    cout << "\nSelected Coupons:\n";
    for (const auto& coupon : selectedCoupons) {
        cout << "ID: " << coupon.id
            << ", Discount: " << coupon.discountValue
            << ", Expiry Date: " << coupon.expiryDate
            << ", Cost Requirement: " << coupon.costRequirement << endl;
    }
    cout << "Final Cost After Discounts: " << finalCost << endl;

    vector<Coupon> remainingCoupons;
    for (const auto& coupon : coupons) {
        bool used = false;
        for (const auto& selected : selectedCoupons) {
            if (coupon.id == selected.id) {
                used = true;
                break;
            }
        }
        if (!used) {
            remainingCoupons.push_back(coupon);
        }
    }

    cout << "\nRemaining Coupons (might expire soon):\n";
    for (const auto& coupon : remainingCoupons) {
        cout << "ID: " << coupon.id
            << ", Discount: " << coupon.discountValue
            << ", Expiry Date: " << coupon.expiryDate
            << ", Cost Requirement: " << coupon.costRequirement << endl;
    }
}

double calculateAverageExpiry(const vector<Coupon>& coupons) {
    if (coupons.empty()) return numeric_limits<double>::max();
    double sum = 0;
    for (const auto& coupon : coupons) {
        sum += coupon.expiryDate;
    }
    return sum / coupons.size();
}

// Greedy Function
vector<Coupon> greedySelectCoupons(vector<Coupon>& coupons, int userCost, int maxCoupons, int leastCost) {
    vector<Coupon> validCoupons;

    for (const auto& coupon : coupons) {
        if (coupon.costRequirement <= userCost) {
            validCoupons.push_back(coupon);
        }
    }

    // Sort the filtered coupons by expiryDate (ascending) and, if equal, by discountValue (descending)
    sort(validCoupons.begin(), validCoupons.end(), [](const Coupon& a, const Coupon& b) {
        if (a.expiryDate != b.expiryDate) {
            return a.expiryDate < b.expiryDate; // Sort by expiryDate (ascending)
        }
        return a.discountValue > b.discountValue; // If expiryDate is equal, sort by discountValue (descending)
        });

    vector<Coupon> selectedCoupons;
    vector<Coupon> closestCoupons;
    double remainingCost = userCost;
    double closestCost = numeric_limits<double>::max();

    // Greedily select coupons
    for (const auto& coupon : validCoupons) {
        if (selectedCoupons.size() < maxCoupons && remainingCost - coupon.discountValue >= leastCost) {
            selectedCoupons.push_back(coupon);
            remainingCost -= coupon.discountValue;

            // Check if the current remainingCost is closer to leastCost
            if (remainingCost >= leastCost && remainingCost < closestCost) {
                closestCost = remainingCost;
                closestCoupons = selectedCoupons;
            }
        }
    }

    // Return the subset with the closest cost to leastCost
    return closestCoupons;
}

// Backtracking
void findOptimalSubset(const vector<Coupon>& coupons, int maxCoupons, int leastCost, double remainingCost,
    vector<Coupon>& currentSubset, vector<Coupon>& bestSubset,
    double& closestCost, double& bestAvgExpiry) {
    double currentAvgExpiry = calculateAverageExpiry(currentSubset);

    if (remainingCost >= leastCost &&
        (remainingCost < closestCost ||
            (remainingCost == closestCost && currentAvgExpiry < bestAvgExpiry))) {
        closestCost = remainingCost;
        bestAvgExpiry = currentAvgExpiry;
        bestSubset = currentSubset;
    }

    if (currentSubset.size() == maxCoupons) {
        return;
    }

    for (size_t i = 0; i < coupons.size(); ++i) {
        const Coupon& coupon = coupons[i];
        double newRemainingCost = remainingCost - coupon.discountValue;

        if (newRemainingCost >= leastCost) {
            currentSubset.push_back(coupon);
            // Recursive call with remaining coupons
            vector<Coupon> remainingCoupons(coupons.begin() + i + 1, coupons.end());
            findOptimalSubset(remainingCoupons, maxCoupons, leastCost, newRemainingCost, currentSubset, bestSubset, closestCost, bestAvgExpiry);
            // Backtrack
            currentSubset.pop_back();
        }
    }
}

// function to handle sorting and initiate backtracking
vector<Coupon> backtrackingSelectCoupons(vector<Coupon>& coupons, int userCost, int maxCoupons, int leastCost) {
    vector<Coupon> validCoupons;
    for (const auto& coupon : coupons) {
        if (coupon.costRequirement <= userCost) {
            validCoupons.push_back(coupon);
        }
    }
    sort(validCoupons.begin(), validCoupons.end(), [](const Coupon& a, const Coupon& b) {
        if (a.expiryDate != b.expiryDate) {
            return a.expiryDate < b.expiryDate; // Sort by expiryDate (ascending)
        }
        return a.discountValue > b.discountValue; // If expiryDate is equal, sort by discountValue (descending)
        });

    vector<Coupon> currentSubset;
    vector<Coupon> bestSubset;
    double closestCost = numeric_limits<double>::max();
    double bestAvgExpiry = numeric_limits<double>::max();
    findOptimalSubset(validCoupons, maxCoupons, leastCost, userCost, currentSubset, bestSubset, closestCost, bestAvgExpiry);

    return bestSubset;
}

//DP
vector<Coupon> dpSelectCoupons(const vector<Coupon>& coupons, int userCost, int maxCoupons, int leastCost) {
    vector<Coupon> validCoupons;
    for (const auto& coupon : coupons) {
        if (coupon.costRequirement <= userCost) {
            validCoupons.push_back(coupon);
        }
    }

    sort(validCoupons.begin(), validCoupons.end(), [](const Coupon& a, const Coupon& b) {
        if (a.expiryDate != b.expiryDate) {
            return a.expiryDate < b.expiryDate;
        }
        return a.discountValue > b.discountValue;
        });

    vector<vector<pair<double, tuple<double, vector<Coupon>>>>> dp(
        maxCoupons + 1, vector<pair<double, tuple<double, vector<Coupon>>>>(userCost + 1, { numeric_limits<double>::max(), {numeric_limits<double>::max(), {}} }));

    dp[0][userCost] = { userCost, {numeric_limits<double>::max(), {}} };

    for (const auto& coupon : validCoupons) {
        for (int usedCoupons = maxCoupons - 1; usedCoupons >= 0; --usedCoupons) {
            for (int currentCost = userCost; currentCost >= 0; --currentCost) {
                if (dp[usedCoupons][currentCost].first != numeric_limits<double>::max()) {
                    double newCost = dp[usedCoupons][currentCost].first - coupon.discountValue;
                    if (newCost >= leastCost) {
                        auto newSelection = get<1>(dp[usedCoupons][currentCost].second);
                        newSelection.push_back(coupon);

                        double newAverageExpiry = calculateAverageExpiry(newSelection);

                        if (newCost < dp[usedCoupons + 1][static_cast<int>(newCost)].first ||
                            (newCost == dp[usedCoupons + 1][static_cast<int>(newCost)].first &&
                                newAverageExpiry < get<0>(dp[usedCoupons + 1][static_cast<int>(newCost)].second))) {
                            dp[usedCoupons + 1][static_cast<int>(newCost)] = { newCost, {newAverageExpiry, newSelection} };
                        }
                    }
                }
            }
        }
    }

    pair<double, tuple<double, vector<Coupon>>> bestSolution = { numeric_limits<double>::max(), {numeric_limits<double>::max(), {}} };
    for (int usedCoupons = 1; usedCoupons <= maxCoupons; ++usedCoupons) {
        for (int currentCost = leastCost; currentCost <= userCost; ++currentCost) {
            if (dp[usedCoupons][currentCost].first < bestSolution.first ||
                (dp[usedCoupons][currentCost].first == bestSolution.first &&
                    get<0>(dp[usedCoupons][currentCost].second) < get<0>(bestSolution.second))) {
                bestSolution = dp[usedCoupons][currentCost];
            }
        }
    }

    return get<1>(bestSolution.second);
}

// function to handle all the questions and coupon management
void manageCoupons(vector<Coupon>& coupons, const string& filename) {

    cout << "\nDo you want to add new coupons or run the program? (Add/Run): ";
    string choice;
    cin >> choice;

    if (choice == "Add") {
        int numCoupons;
        cout << "How many coupons would you like to add? ";
        cin >> numCoupons;

        for (int i = 0; i < numCoupons; ++i) {
            int id, expiry, costReq;
            double discount;
            cout << "Enter coupon ID: ";
            cin >> id;
            cout << "Enter discount value: ";
            cin >> discount;
            cout << "Enter expiry date (YYYYMMDD): ";
            cin >> expiry;
            cout << "Enter cost requirement: ";
            cin >> costReq;

            coupons.push_back(Coupon(id, discount, expiry, costReq));
        }

        // Save the newly added coupons to the file
        saveCoupons(coupons, filename);
    }

    int LeastCost, CustomerScore, userCost;
    vector<Coupon> selectedCoupons;

    cout << "Enter your total cost: ";
    cin >> userCost;
    cout << "Enter the least cost you should pay: ";
    cin >> LeastCost;
    cout << "Enter your customer score (representing your max coupons number to be used): ";
    cin >> CustomerScore;

    cout << "\nDo you prefer not losing any coupons that might end soon and still get max savings or you would prefer better max savings today but lose coupons that might end soon?\n";
    cout << "ANSWER 1 or 2: ";
    int choiceNum;
    cin >> choiceNum;

    if (choiceNum == 1) {
        cout << "You chose to keep coupons that might expire soon.\n";
        selectedCoupons = greedySelectCoupons(coupons, userCost, CustomerScore, LeastCost);
    }
    else if (choiceNum == 2) {
        cout << "You chose to maximize savings today.\n";
        cout << "Which algorithm would you like to use? (BT/DP): ";
        string algorithmChoice;
        cin >> algorithmChoice;
        if (algorithmChoice == "BT") {
            selectedCoupons = backtrackingSelectCoupons(coupons, userCost, CustomerScore, LeastCost);
        }
        else {
            selectedCoupons = dpSelectCoupons(coupons, userCost, CustomerScore, LeastCost);
        }
    }
    else {
        cout << "Invalid choice. Exiting...\n";
        return;
    }
    displayCoupons(coupons, selectedCoupons, userCost);

    cout << "\nDo you want to proceed with the selected coupons? (Y/N): ";
    char proceedChoice;
    cin >> proceedChoice;

    if (proceedChoice == 'Y' || proceedChoice == 'y') {
        cout << "Proceeding with the selected coupons...\n";
        vector<Coupon> remainingCoupons;
        for (const auto& coupon : coupons) {
            bool used = false;
            for (const auto& selected : selectedCoupons) {
                if (coupon.id == selected.id) {
                    used = true;
                    break;
                }
            }
            if (!used) {
                remainingCoupons.push_back(coupon);
            }
        }
        coupons = remainingCoupons;
        saveCoupons(coupons, filename);
    }

    cout << "Do you want to exit or run again? (Exit/Run): ";
    string finalChoice;
    cin >> finalChoice;

    if (finalChoice == "Exit") {
        cout << "Exiting the program...\n";
        return;
    }
    else {
        manageCoupons(coupons, filename);
    }
}

int main() {
    string filename = "coupons.txt";
    vector<Coupon> coupons = loadCoupons(filename);
    int todayDate;
    cout << "Enter today's date in YYYYMMDD format (e.g., 20240101): ";
    cin >> todayDate;

    removeExpiredCoupons(coupons, todayDate);

    manageCoupons(coupons, filename);

    return 0;
}
