#include "SimDraft.h"


int main()
{
  std::cout << "Select an option; 1 = run draft, 2 = get Big Board for selected team: \n";
  int choice{};
  std::cin >> choice; 

  if (choice == 1) { RunDraft(); }
  else if (choice == 2) { getBigBoard(); };

    return 0;
}
