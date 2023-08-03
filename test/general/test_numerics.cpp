#include "gtest/gtest.h"
#include "cantera/numerics/polyfit.h"
#include "cantera/numerics/funcs.h"
#include "cantera/numerics/Func1Factory.h"

using namespace Cantera;

double polyval(vector<double>& coeffs, double x) {
    double sum = 0;
    double xn = 1;
    for (size_t i = 0; i < coeffs.size(); i++) {
        sum += coeffs[i] * xn;
        xn *= x;
    }
    return sum;
}

TEST(Polyfit, exact_fit)
{
    vector<double> x{0, 0.3, 1.0, 1.5, 2.0, 2.5};
    vector<double> p(6);
    vector<double> w(6, -1.0);
    for (int i = 0; i < 20; i++) {
        vector<double> y{-1.1*i, cos(i), pow(-1,i), 3.2/(i+1), 0.1*i*i, sin(i)};
        polyfit(6, 5, x.data(), y.data(), w.data(), p.data());
        for (size_t j = 0; j < 6; j++) {
            EXPECT_NEAR(polyval(p, x[j]), y[j], 1e-10);
        }
    }
}

TEST(Polyfit, sequential)
{
    vector<double> x{-1.0, 0.0, 0.5, 1.0, 1.5, 2.0, 3.0};
    vector<double> y{0.6, 1.0, 0.8, 0.4, -0.1, -0.5, -1};

    // Coefficients calculated using Numpy's polyfit function for polynomials
    // of degrees 0 - 5.
    std::vector<vector<double>> PP{
        {0.17142857142857154},
        {0.66190476190476177, -0.49047619047619029},
        {0.73605442176870761, -0.19387755102040838, -0.14829931972789107},
        {1.0095838335334129, -0.22426970788315401, -0.51300520208083311,
         0.12156862745098072},
        {1.0121336003688943, -0.23102395749454527, -0.51552488317194212,
         0.12746543334778632, -0.0014742014742014889},
        {0.99812799812799835, -0.093488943488944404, -0.61193011193011071,
         0.011452361452361514, 0.10963690963690906, -0.022222222222222105}
    };

    double rms_prev = 1e10;
    for (size_t i = 0; i < PP.size(); i++) {
        size_t N = i + 1;
        vector<double> p(N);
        double rms = polyfit(7, i, x.data(), y.data(), nullptr, p.data());
        EXPECT_LT(rms, rms_prev);
        rms_prev = rms;
        for (size_t j = 0; j < N; j++) {
            EXPECT_NEAR(PP[i][j], p[j], 1e-14);
        }
    }
}

TEST(Polyfit, weighted)
{
    vector<double> x{-1.0, 0.0, 0.5, 1.0, 1.5, 2.0, 3.0};
    vector<double> y{0.6, 1.0, 0.8, 0.4, -0.1, -0.5, -1};
    vector<double> w{25, 1, 1, 1, 1, 1, 100}; // these are the squares of Numpy's weights

    // Coefficients calculated using Numpy's polyfit function for polynomials
    // of degrees 0 - 5.
    std::vector<vector<double>> PP{
        {-0.64153846153846139},
        {0.24582603619381152, -0.41199065966141246},
        {0.64897277949822718, -0.10796777523450461, -0.14749113594542437},
        {1.0095165556633916, -0.22435606362053356, -0.51254844673169053,
         0.12135217568551074},
        {1.0121717322829622, -0.23147507683766383, -0.51492677362711337,
         0.12728869689006062, -0.0014837700620763492},
        {0.998127784554808, -0.093474983983779111, -0.61196784469972776,
         0.011482911646053995, 0.10962944760868476, -0.022222284629403764}
    };

    double rms_prev = 1e10;
    for (size_t i = 0; i < PP.size(); i++) {
        size_t N = i + 1;
        vector<double> p(N);
        double rms = polyfit(7, i, x.data(), y.data(), w.data(), p.data());
        EXPECT_LT(rms, rms_prev);
        rms_prev = rms;
        for (size_t j = 0; j < N; j++) {
             EXPECT_NEAR(PP[i][j], p[j], 1e-14);
        }
    }
}

TEST(Trapezoidal, four_points)
{
    Eigen::VectorXd x(4);
    x << 0, 0.3, 1.0, 1.2;
    Eigen::VectorXd f(4);
    f << 1.0, 2.0, 5.0, 0.0;
    // This data was generated by numpy.trapz(f,x)
    EXPECT_NEAR(trapezoidal(f, x), 3.4, 1e-5);
    EXPECT_NEAR(numericalQuadrature("trapezoidal", f, x),
                trapezoidal(f, x), 1e-10);
}

TEST(Simpson, odd)
{
    Eigen::VectorXd x(3);
    x << 0, 0.3, 1.0;
    Eigen::VectorXd f(3);
    f << 1.0, 2.0, 5.0;
    // This data was generated by scipy.integrate.simpson(f, x)
    EXPECT_NEAR(simpson(f, x), 2.84127, 1e-5);
    EXPECT_NEAR(numericalQuadrature("simpson", f, x), simpson(f, x), 1e-10);
}

TEST(Simpson, even)
{
    Eigen::VectorXd x(4);
    x << 0, 0.3, 1.0, 1.2;
    Eigen::VectorXd f(4);
    f << 1.0, 2.0, 5.0, 0.0;
    // This data was generated by scipy.integrate.simpson(f, x, even='first')
    EXPECT_NEAR(simpson(f, x), 3.34127, 1e-5);
}

TEST(ctfunc, functor)
{
    auto functor = newFunc1("functor");
    ASSERT_EQ(functor->type(), "functor");
    ASSERT_THROW(functor->derivative3(), CanteraError);
}

TEST(ctfunc, invalid)
{
    // exceptions return -1
    ASSERT_THROW(newFunc1("spam"), CanteraError);
    vector<double> pars = {1., 2.};
    ASSERT_THROW(newFunc1("eggs", pars), CanteraError);
}

TEST(ctfunc, sin)
{
    double omega = 2.;
    auto functor = newFunc1("sin", omega);
    ASSERT_EQ(functor->type(), "sin");
    EXPECT_DOUBLE_EQ(functor->eval(0.), 0.);
    EXPECT_DOUBLE_EQ(functor->eval(.5), sin(omega * .5));

    auto dfunctor = functor->derivative3();
    EXPECT_DOUBLE_EQ(dfunctor->eval(0.), omega);
    EXPECT_DOUBLE_EQ(dfunctor->eval(.5), omega * cos(omega * .5));

    ASSERT_THROW(newFunc1("sin", vector<double>()), CanteraError);
}

TEST(ctfunc, cos)
{
    double omega = 2.;
    auto functor = newFunc1("cos", omega);
    ASSERT_EQ(functor->type(), "cos");
    EXPECT_DOUBLE_EQ(functor->eval(0.), 1.);
    EXPECT_DOUBLE_EQ(functor->eval(.5), cos(omega * .5));

    auto dfunctor = functor->derivative3();
    EXPECT_DOUBLE_EQ(dfunctor->eval(0.), 0.);
    EXPECT_DOUBLE_EQ(dfunctor->eval(.5), -omega * sin(omega * .5));

    ASSERT_THROW(newFunc1("cos", {1., 2.}), CanteraError);
}

TEST(ctfunc, exp)
{
    double omega = 2.;
    auto functor = newFunc1("exp", omega);
    ASSERT_EQ(functor->type(), "exp");
    EXPECT_DOUBLE_EQ(functor->eval(0.), 1.);
    EXPECT_DOUBLE_EQ(functor->eval(.5), exp(omega * .5));

    auto dfunctor = functor->derivative3();
    EXPECT_DOUBLE_EQ(dfunctor->eval(0.), omega);
    EXPECT_DOUBLE_EQ(dfunctor->eval(.5), omega * exp(omega * .5));

    ASSERT_THROW(newFunc1("exp", {1., 2.}), CanteraError);
}

TEST(ctfunc, log)
{
    double omega = 2.;
    auto functor = newFunc1("log", omega);
    ASSERT_EQ(functor->type(), "log");
    EXPECT_DOUBLE_EQ(functor->eval(0.1), log(omega * .1));
    EXPECT_DOUBLE_EQ(functor->eval(1. / omega), 0.);
    EXPECT_DOUBLE_EQ(functor->eval(10.), log(omega * 10.));

    auto dfunctor = functor->derivative3();
    EXPECT_DOUBLE_EQ(dfunctor->eval(.1), omega / .1);
    EXPECT_DOUBLE_EQ(dfunctor->eval(.5), omega / .5);

    ASSERT_THROW(newFunc1("log", vector<double>()), CanteraError);
}

TEST(ctfunc, pow)
{
    double exp = .5;
    auto functor = newFunc1("pow", exp);
    EXPECT_DOUBLE_EQ(functor->eval(0.), 0.);
    EXPECT_DOUBLE_EQ(functor->eval(.5), pow(.5, exp));

    auto dfunctor = functor->derivative3();
    EXPECT_DOUBLE_EQ(dfunctor->eval(.5), exp * pow(.5, exp - 1));

    ASSERT_THROW(newFunc1("pow", vector<double>()), CanteraError);
}

TEST(ctfunc, constant)
{
    double a = .5;
    auto functor = newFunc1("constant", a);
    EXPECT_EQ(functor->type(), "constant");
    EXPECT_DOUBLE_EQ(functor->eval(0.), a);
    EXPECT_DOUBLE_EQ(functor->eval(.5), a);

    auto dfunctor = functor->derivative3();
    EXPECT_DOUBLE_EQ(dfunctor->eval(0.), 0.);
    EXPECT_DOUBLE_EQ(dfunctor->eval(.5), 0.);

    ASSERT_THROW(newFunc1("constant", {1., 2., 3.}), CanteraError);
}

TEST(ctfunc, tabulated_linear)
{
    vector<double> params = {0., 1., 2., 1., 0., 1.};

    auto functor = newFunc1("tabulated-linear", params);
    ASSERT_EQ(functor->type(), "tabulated-linear");
    EXPECT_DOUBLE_EQ(functor->eval(0.), 1.);
    EXPECT_DOUBLE_EQ(functor->eval(.5), .5);
    EXPECT_DOUBLE_EQ(functor->eval(1.), 0.);
    EXPECT_DOUBLE_EQ(functor->eval(1.2), 0.2);
    EXPECT_DOUBLE_EQ(functor->eval(2.), 1.);

    auto dfunctor = functor->derivative3();
    EXPECT_DOUBLE_EQ(dfunctor->eval(.5), -1.);
    EXPECT_DOUBLE_EQ(dfunctor->eval(1.5), 1.);

    params.push_back(1.);
    ASSERT_THROW(newFunc1("tabulated-linear", params), CanteraError);
}

TEST(ctfunc, tabulated_previous)
{
    vector<double> params = {0., 1., 2., 1., 0., 1.};

    auto functor = newFunc1("tabulated-previous", params);
    ASSERT_EQ(functor->type(), "tabulated-previous");
    EXPECT_DOUBLE_EQ(functor->eval(0.), 1.);
    EXPECT_DOUBLE_EQ(functor->eval(.5), 1.);
    EXPECT_DOUBLE_EQ(functor->eval(1. - 1.e-12), 1.);
    EXPECT_DOUBLE_EQ(functor->eval(1. + 1.e-12), 0.);
    EXPECT_DOUBLE_EQ(functor->eval(1.2), 0.);
    EXPECT_DOUBLE_EQ(functor->eval(2. - 1.e-12), 0.);
    EXPECT_DOUBLE_EQ(functor->eval(2. + 1.e-12), 1.);

    auto dfunctor = functor->derivative3();
    EXPECT_DOUBLE_EQ(dfunctor->eval(.5), 0.);
    EXPECT_DOUBLE_EQ(dfunctor->eval(1.5), 0.);
}

TEST(ctfunc, poly)
{
    double a0 = .5;
    double a1 = .25;
    double a2 = .125;
    vector<double> params = {a0, a1, a2};
    auto functor = newFunc1("polynomial", params);
    ASSERT_EQ(functor->type(), "polynomial");
    EXPECT_DOUBLE_EQ(functor->eval(0.), a0);
    EXPECT_DOUBLE_EQ(functor->eval(.5), (a2 * .5 + a1) * .5 + a0);

    ASSERT_THROW(functor->derivative3(), CanteraError);
}

TEST(ctfunc, Fourier)
{
    double a0 = .5;
    double a1 = .25;
    double b1 = .125;
    double omega = 2.;
    vector<double> params = {a0, a1, omega, b1};
    auto functor = newFunc1("Fourier", params);
    ASSERT_EQ(functor->type(), "Fourier");
    EXPECT_DOUBLE_EQ(functor->eval(0.), .5 * a0 + a1);
    EXPECT_DOUBLE_EQ(
        functor->eval(.5), .5 * a0 + a1 * cos(omega * .5) + b1 * sin(omega * .5));

    ASSERT_THROW(functor->derivative3(), CanteraError);

    params.push_back(1.);
    ASSERT_THROW(newFunc1("Fourier", params), CanteraError);
    ASSERT_THROW(newFunc1("Fourier", vector<double>({1., 2.})), CanteraError);
}

TEST(ctfunc, Gaussian)
{
    double A = .5;
    double t0 = .6;
    double fwhm = .25;
    vector<double> params = {A, t0, fwhm};
    auto functor = newFunc1("Gaussian", params);
    ASSERT_EQ(functor->type(), "Gaussian");
    double tau = fwhm / (2. * sqrt(log(2.)));
    double x = - t0 / tau;
    EXPECT_DOUBLE_EQ(functor->eval(0.), A * exp(-x * x));
    x = (.5 - t0) / tau;
    EXPECT_DOUBLE_EQ(functor->eval(.5), A * exp(-x * x));

    ASSERT_THROW(functor->derivative3(), CanteraError);

    ASSERT_THROW(newFunc1("Gaussian", vector<double>({1., 2.})), CanteraError);
}

TEST(ctfunc, Arrhenius)
{
    double A = 38.7;
    double b = 2.7;
    double E = 2.619184e+07 / GasConstant;
    vector<double> params = {A, b, E};
    auto functor = newFunc1("Arrhenius", params);
    ASSERT_EQ(functor->type(), "Arrhenius");
    EXPECT_DOUBLE_EQ(functor->eval(1000.), A * pow(1000., b) * exp(-E/1000.));

    ASSERT_THROW(functor->derivative3(), CanteraError);

    ASSERT_THROW(newFunc1("Arrhenius", vector<double>({1., 2.})), CanteraError);
}

TEST(ctmath, invalid)
{
    auto functor0 = newFunc1("sin");
    auto functor1 = newFunc1("cos");
    ASSERT_THROW(newFunc1("foo", functor0, functor1), CanteraError);
    ASSERT_THROW(newFunc1("bar", functor0, 0.), CanteraError);
}

TEST(ctmath, sum)
{
    double omega = 2.;
    auto functor0 = newFunc1("sin", omega);
    auto functor1 = newFunc1("cos", omega);
    auto functor = newFunc1("sum", functor0, functor1);
    EXPECT_EQ(functor->type(), "sum");
    EXPECT_DOUBLE_EQ(functor->eval(0.), 1.);
    EXPECT_DOUBLE_EQ(functor->eval(.5), sin(omega * .5) + cos(omega * .5));

    auto dfunctor = functor->derivative3();
    EXPECT_DOUBLE_EQ(dfunctor->eval(.5), omega * (cos(omega * .5) - sin(omega * .5)));
}

TEST(ctmath, diff)
{
    double omega = 2.;
    auto functor0 = newFunc1("sin", omega);
    auto functor1 = newFunc1("cos", omega);
    auto functor = newFunc1("diff", functor0, functor1);
    EXPECT_EQ(functor->type(), "diff");
    EXPECT_DOUBLE_EQ(functor->eval(0.), -1.);
    EXPECT_DOUBLE_EQ(functor->eval(.5), sin(omega * .5) - cos(omega * .5));

    auto dfunctor = functor->derivative3();
    EXPECT_DOUBLE_EQ(dfunctor->eval(.5), omega * (cos(omega * .5) + sin(omega * .5)));
}

TEST(ctmath, prod)
{
    double omega = 2.;
    auto functor0 = newFunc1("sin", omega);
    auto functor1 = newFunc1("cos", omega);
    auto functor = newFunc1("product", functor0, functor1);
    EXPECT_EQ(functor->type(), "product");
    EXPECT_DOUBLE_EQ(functor->eval(0.), 0);
    EXPECT_DOUBLE_EQ(functor->eval(0.5), sin(omega * 0.5) * cos(omega * 0.5));

    auto dfunctor = functor->derivative3();
    EXPECT_DOUBLE_EQ(dfunctor->eval(0.5),
        omega * (pow(cos(omega * 0.5), 2) - pow(sin(omega * 0.5), 2)));
}

TEST(ctmath, ratio)
{
    double omega = 2.;
    auto functor0 = newFunc1("sin", omega);
    auto functor1 = newFunc1("cos", omega);
    auto functor = newFunc1("ratio", functor0, functor1);
    EXPECT_EQ(functor->type(), "ratio");
    EXPECT_DOUBLE_EQ(functor->eval(0.), 0.);
    EXPECT_DOUBLE_EQ(functor->eval(0.5), sin(omega * 0.5) / cos(omega * 0.5));

    auto dfunctor = functor->derivative3();
    EXPECT_DOUBLE_EQ(dfunctor->eval(0.5), omega / pow(cos(omega * 0.5), 2));
}

TEST(ctmath, composite)
{
    double omega = 2.;
    auto functor0 = newFunc1("sin", omega);
    auto functor1 = newFunc1("cos", omega);
    auto functor = newFunc1("composite", functor0, functor1);
    EXPECT_EQ(functor->type(), "composite");
    EXPECT_DOUBLE_EQ(functor->eval(0.), sin(omega));
    EXPECT_DOUBLE_EQ(functor->eval(0.5), sin(omega * cos(omega * 0.5)));

    auto dfunctor = functor->derivative3();
    EXPECT_DOUBLE_EQ(dfunctor->eval(0.5),
        - omega * omega * sin(omega * 0.5) * cos(omega * cos(omega * 0.5)));
}

TEST(ctmath, times_constant)
{
    double omega = 2.;
    auto functor0 = newFunc1("sin", omega);
    double A = 1.234;
    auto functor = newFunc1("times-constant", functor0, A);
    EXPECT_EQ(functor->type(), "times-constant");
    EXPECT_DOUBLE_EQ(functor->eval(0.), 0.);
    EXPECT_DOUBLE_EQ(functor->eval(0.5), sin(omega * 0.5) * A);

    auto dfunctor = functor->derivative3();
    EXPECT_DOUBLE_EQ(dfunctor->eval(0.5), A * omega * cos(omega * 0.5));
}

TEST(ctmath, plus_constant)
{
    double omega = 2.;
    auto functor0 = newFunc1("sin", omega);
    double A = 1.234;
    auto functor = newFunc1("plus-constant", functor0, A);
    EXPECT_EQ(functor->type(), "plus-constant");
    EXPECT_DOUBLE_EQ(functor->eval(0.), A);
    EXPECT_DOUBLE_EQ(functor->eval(0.5), sin(omega * 0.5) + A);

    auto dfunctor = functor->derivative3();
    EXPECT_DOUBLE_EQ(dfunctor->eval(0.5), omega * cos(omega * 0.5));
}

TEST(ctmath, periodic)
{
    double omega = 2.;
    auto functor0 = newFunc1("sin", omega);
    double A = 1.234;
    auto functor = newFunc1("periodic", functor0, A);
    EXPECT_EQ(functor->type(), "periodic");
    EXPECT_DOUBLE_EQ(functor->eval(0.), functor->eval(A));
    EXPECT_DOUBLE_EQ(functor->eval(0.5), functor->eval(0.5 + A));

    ASSERT_THROW(functor->derivative3(), CanteraError);
}
