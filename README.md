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

### 2. Retirement service `RetirementService`

A service that calculates employees' retirement eligibility, theoretical retirement dates, and estimated monthly pensions.

| Function | Description | Input | Output |
|---|---|---|---|
| `RetirementService(...)` | Initialize the service and its retirement rules | `id`, `name`, `description`, `retirementAge`, `minimumYearsOfService`, `pensionRate`, `maximumPensionRate` | `RetirementService` instance |
| `calculateAge` | Calculate the employee's current age using their date of birth | `const Employe&` | `int` age |
| `calculateYearsOfService` | Calculate the employee's seniority in years | `const Employe&` | `int` years |
| `isEligibleForRetirement` | Check whether the employee meets the minimum retirement age and seniority requirements | `const Employe&` | `bool` eligibility |
| `calculateRetirementDate` | Calculate the theoretical date when the employee reaches retirement age | `const Employe&` | `Date` |
| `estimateMonthlyPension` | Estimate the employee's monthly pension based on their base salary and years of service, applying the maximum pension rate | `const Employe&` | `double` amount |
| `getRetirementAge`, `getMinimumYearsOfService`, `getPensionRate`, `getMaximumPensionRate` | Return the retirement service configuration | none | `int`, `int`, `double`, `double` |
| `setRetirementAge`, `setMinimumYearsOfService`, `setPensionRate`, `setMaximumPensionRate` | Update the retirement rules | new value | `void` |
| `display` | Print the retirement service configuration | none | console output |
| `execute` | List employees, prompt for an employee ID, check retirement eligibility, and display the theoretical retirement date and estimated monthly pension when eligible | standard input | console output |