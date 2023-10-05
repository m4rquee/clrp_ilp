#include "gurobi_c++.h"

using namespace std;

void printSolution(GRBModel &model, GRBVar **X, int n) { // Print the edges of the generated graph
    if (model.get(GRB_IntAttr_Status) != GRB_OPTIMAL) {
        cout << "No solution" << endl;
        return;
    }

    cout << "Edges: ";
    for (int i = 0; i < n; i++)
        for (int j = i + 1; j < n; j++) {
            if (X[i][j].get(GRB_DoubleAttr_X) > 0.0001)
                cout << i << "-" << j << ", ";
        }
    cout << endl;
}

void readInput(bool **&L_part, int *&L_values, int &n, int &m) {
    cin >> n >> m;
    L_part = new bool *[m];
    L_values = new int[m];
    int part_size, aux;
    for (int i = 0; i < m; i++) {
        L_part[i] = static_cast<bool *>(calloc(n, sizeof(bool)));
        cin >> part_size >> L_values[i];
        for (int j = 0; j < part_size; j++) {
            cin >> aux;
            L_part[i][aux] = true;
        }
    }
}

int main() {
    int n, m;
    bool **L_part;// List of partitions
    int *L_values;// List of cut sizes
    readInput(L_part, L_values, n, m);

    try {
        // Model
        auto env = new GRBEnv();
        auto model = GRBModel(*env);
        model.set(GRB_StringAttr_ModelName, "Cut List Realization Problem");
        model.set(GRB_IntAttr_ModelSense, GRB_MINIMIZE);

        // Create boolean decision variables for the edges
        char name[100];
        auto **X = new GRBVar *[n];
        for (int i = 0; i < n; i++) {
            X[i] = new GRBVar[n];
            for (int j = 0; j < n; j++) {
                if (i > j)
                    X[i][j] = X[j][i]; // just copy the other matrix half
                else if (i != j) {
                    sprintf(name, "x_%d,%d", i, j);
                    X[i][j] = model.addVar(0.0, 1.0, 1.0, GRB_BINARY, name);
                }
            }
        }

        // Create the remaining edges per cut variables (used for the error minimizing version)
        /*GRBVar Y[m];
        for (int l = 0; l < m; l++) {
            sprintf(name, "y_%d", l);
            Y[l] = model.addVar(0.0, L_values[l], 1.0, GRB_INTEGER, name);
        }*/

        model.update();

        // Cut constraints
        for (int l = 0; l < m; l++) {
            GRBLinExpr cut_constr = 0;
            for (int u = 0; u < n; u++)
                for (int v = u + 1; v < n; v++)
                    if (L_part[l][u] != L_part[l][v]) // an edge of the cut induced by partition l
                        cut_constr += X[u][v];

            model.addConstr(cut_constr == L_values[l]);

            // Min violations (used for the error minimizing version)
            // model.addConstr(cut_constr <= L_values[l]); // The number of edges between these parts are limited
            // model.addConstr(cut_constr + Y[l] == L_values[l]); // Added edges plus the slack variable is the total
        }

        // Solve
        model.optimize();
        model.write("clrp.lp");
        printSolution(model, X, n);

        delete env;
    } catch (GRBException e) {
        cout << "Error code = " << e.getErrorCode() << endl;
        cout << e.getMessage() << endl;
    } catch (...) {
        cout << "Exception during optimization" << endl;
    }

    return 0;
}
