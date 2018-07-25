# U.S. Temperature Analysis
Get statistical information regarding temperatures and dew points at various weather stations across the United States.

# Motivation
To allow for more advanced probabilistic analysis of extreme temperature likelihoods, as well as additional statistical information
that is not normally available elsewhere.  Get tenth to ninetieth percentile dew point ranges for various time periods.  Also provides a speedy way to switch between various stations and get a detailed sense of their temperature trends, with all the features in one consolidated module.

# Code Style
C Standard

# Software Development Kit
Windows 10 SDK

# Platform Toolset
Visual Studio 2017

# Features
## Yearly and Monthly History Lookup
![alt text](https://github.com/Silveresque/USTemperatureAnalysis/blob/master/Screenshots/Historical%20Records.png)

## Record and Mean Extrema Tables
![alt text](https://github.com/Silveresque/USTemperatureAnalysis/blob/master/Screenshots/Record%20and%20Mean%20Extrema%20Table.png)

## Percentile Tables
![alt text](https://github.com/Silveresque/USTemperatureAnalysis/blob/master/Screenshots/Percentile%20Table.png)

## Daily Normals
![alt text](https://github.com/Silveresque/USTemperatureAnalysis/blob/master/Screenshots/Daily%20Normals.png)

## Hourly Normals
![alt text](https://github.com/Silveresque/USTemperatureAnalysis/blob/master/Screenshots/Hourly%20Normals.png)

## Probability of Extreme Temperatures
![alt text](https://github.com/Silveresque/USTemperatureAnalysis/blob/master/Screenshots/Extreme%20Probabilities.png)

## Simulation of Maximum and Minimum Temperatures in a Month or Whole Year
![alt text](https://github.com/Silveresque/USTemperatureAnalysis/blob/master/Screenshots/Simulation.png)

## Dew Point Annual Summary
![alt text](https://github.com/Silveresque/USTemperatureAnalysis/blob/master/Screenshots/Dew%20Point%20Annual%20Table.png)

## Daily Dew Points
![alt text](https://github.com/Silveresque/USTemperatureAnalysis/blob/master/Screenshots/Daily%20Dew%20Point.png)

## Hourly Dew Points
![alt text](https://github.com/Silveresque/USTemperatureAnalysis/blob/master/Screenshots/Hourly%20Dew%20Point.png)

## Gumbel Probability Distribution
The gumbel probability distribution is used to evaluate probabilities and percentiles for maximums and minimums in a specific month or the whole year.  

A probability density function (pdf) produces a contiuous curve on the graph that specifies different probabilities (y-values) for corresponding values (x-values).  Because dx, or the change in x, can be infinitely small, the probability of a specific value will always be 0.  Therefore, probabilites from a (pdf) are typically evaluated by integrating over the pdf over a certain range and calculating the area under the curve in that range.  

For example, the probability of a value between 60 and 70 occurring in a probability distribution, could be evaluated by taking the definite integral of its pdf from x = 60 to x = 70.

The cumultaive distribution function (cdf), derived from the pdf, calculates the probability of an occurrence less than (the "or equal to" distintion becomes interchangeable and negllibile due to the nature of pdf's explianed above) a specified value.  Therefore, the cdf is derived by evaluating the definite integral from negative infinity to x of the pdf.  As an example, to evaluate the probability of the occurrence of a value less than 40 in a probability distribution, the cdf evaluated at 40 would calculate this probability.

The survival function is the opposite of the cdf, calculating the probaiblity of an occurrence greater than ( and optionally "or equal to") a specified value.  If the probability of event A is p, the probability of event A' is 1 - p.  Therefore, the survival function is simply equal to 1 minus the cdf.  As an example, to evaluate the probability of the occurrence of a value greater than 80 in a probability distribution, the survival function evaluated at 80 would calculate this probability.

For the purposes of this project, the distinction betwen cdf and survival function is not made.  In this project, the cdf of the maximum gumbel evaluated at x, outputs the probability of occurrence x or greater.  The cdf of the minimum gumbel evaluated at x, outputs the probability of occurrence x or lower.

Gumbel distributions are used to calculate probabilities such as, "What is the probability of seeing a temperature 100 degrees fahrenheit in July?" The maximum gumbel cdf (survival function behind the scenes) evaluated at 100 is used for such a calculation.  This outputs the probability of seeing a temperature value 100 or greater, which is the desired result, as the occurrence of any greater temperature value will include the occurrence of 100 degrees (for example, an occurrence of 104 degrees had to pass 100 degrees in an upward direction).  Probabilities like, "What is the probability of seeing a temperature 0 degrees fahrenheit in January?" are evaluated similarly, but by using the cdf of the minimum gumbel.

Taking the inverse of these functions is used to generate a value given a percentile.  This is used for the simulation features in this project, by generating random percentiles and plugging them into these inverse functions.  

### Geometric Probability Distribution
In this project, gumbel distributions are used to evaluate the probabilities of certain temperature values occurring in one month or one year.  By calculating the probability of occurrence in one period, we can then use this as the p-value for the well-known geometric distribution, which can then be used to calculate the probability of the value's occurrence over multiple periods.

Example: The probability of seeing a 100 degree day (or greater) at a location in one July is 25% (and thus the probability of not seeing its ocucurrence in one July is 75%).  Using the geometric distribution, the probability of seeing a 100 degree day at a location at least once in 5 July's is 1 - 0.75^5 (1 minus the probability of not seeing at all in each of the 5 years) is 76%.  This concept is used to calculate the 1-, 5-, 10-, 15-, 20-, and 25-year probabilities of the occurrence of extreme temperature values.

## Dew Point Information
Dew point is often a better indicator of humidity as humany perceived, compared to relative humidity.  This is because, dew points evaluate the actual amount of moisture in the air, whereas relative humidity indicates how much moisture is in the air relative to how much can be present.  For example, 50% relative humidity at 90 degrees fahrenheit yields a dew point of 69 degrees, but 50% relative humidity at 50 degrees yields a dew point of 32 degrees.  Here's an idea of how various dew points are perceived by the average person:

Dew Point (degrees fahrenheit) | Perception

      >75 | Amazon Rainforest
      
       70 | Very Humid
       
       60 | Humid
       
       50 | Comfortable
       
       40 | Brisk
       
      <35 | Dry
      
As you can see, the same relative humidity at variuos temperatures can yield vastly different actual amounts of moisture.  Thereofre, dew point, a variable that instead is based on the actual amount of moisture, is a better indicator of perception.  
 
# Works Cited
Menne, M.J., I. Durre, R.S. Vose, B.E. Gleason, and T.G. Houston, 2012:  An overview
of the Global Historical Climatology Network - Daily Database.  Journal of Atmospheric
and Oceanic Technology, 29, 897 - 910, doi:10.1175 / JTECH - D - 11 - 00103.1.
Menne, M.J., I.Durre, B.Korzeniewski, S.McNeal, K.Thomas, X.Yin, S.Anthony, R.Ray,

R.S.Vose, B.E.Gleason, and T.G.Houston, 2012: Global Historical Climatology Network -
Daily(GHCN - Daily), Version 3.25
NOAA National Climatic Data Center.http ://doi.org/10.7289/V5D21VHZ 6 Jun. 2018.

Anthony Arguez, Imke Durre, Scott Applequist, Mike Squires, Russell Vose, Xungang Yin, and Rocky Bilotta(2010).
NOAA's U.S. Climate Normals (1981-2010). Hourly.
NOAA National Centers for Environmental Information. DOI:10.7289/V5PN93JP 27 Dec. 2017.

Anthony Arguez, Imke Durre, Scott Applequist, Mike Squires, Russell Vose, Xungang Yin, and Rocky Bilotta(2010).
NOAA's U.S. Climate Normals (1981-2010). Temperature.
NOAA National Centers for Environmental Information. DOI:10.7289/V5PN93JP 9 Jul. 2018.

# Authors
Sean Datta
