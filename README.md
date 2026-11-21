# Systeme-Gestion-Employees
Employee Management System (EMS) built using C++.
> It will be good if everyone can make a table like this to describe the functions of their service. In order to make it easier for integration and testing.
### 1.Prime service `Bonification`
 A service that calculates bonuses for employees : 

| Function | Description | Input | Output |
|---|---|---|---|
| `PrimeService(...)` | Initialize the service and its bonus rules | `id`, `GestionnaireEmployes&`, `name`, `description`, `tauxParAnnee`, `tauxMaximum`, `ancienneteMinimum` | `PrimeService` instance |
| `calculerTaux` | Return the seniority rate capped at the maximum | `anneesAnciennete` | `double` rate |
| `calculerMontant` | Return the bonus amount rounded to cents | `const Employe&` | `double` amount |
| `bonifier` | Compute the bonus for an employee | `const Employe&` | `Bonification` |
| `getTauxParAnnee`, `getTauxMaximum`, `getAncienneteMinimum` | Return the bonus rules | none | `double`, `double`, `int` |
| `setTauxParAnnee`, `setTauxMaximum`, `setAncienneteMinimum` | Validate and update the bonus rules | new value | `void` |
| `display` | Print the service configuration | none | console output |
| `execute` | List employees, prompt for an id and apply the bonus | standard input | console output |
