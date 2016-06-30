/**
 * BMS project 2
 * Fridolin Pokorny <fridex.devel@gmail.com>
 *
 * @date 2013/12/05
 */

#ifndef TABLE_H_
#define TABLE_H_

// http://en.wikipedia.org/wiki/DVB-T
double TSstream::getTotalBitRate() {
    double ret = 0.;
    switch (m_nit_constellation) {
        case 0 /*QPSK*/:
            switch (m_nit_code_rate) {
                case 0/*1/2*/:
                    switch (m_nit_guard) {
                        case 3/*1/4*/:
                            ret = 4.976;
                            break;
                        case 2/*1/8*/:
                            ret = 5.529;
                            break;
                        case 1/*1/16*/:
                            ret = 5.855;
                            break;
                        case 0/*1/32*/:
                            ret = 6.032;
                            break;
                    }
                    break;
                case 1/*2/3*/:
                    switch (m_nit_guard) {
                        case 3/*1/4*/:
                            ret = 6.635;
                            break;
                        case 2/*1/8*/:
                            ret = 7.373;
                            break;
                        case 1/*1/16*/:
                            ret = 7.806;
                            break;
                        case 0/*1/32*/:
                            ret = 8.043;
                            break;
                    }
                    break;
                case 2/*3/4*/:
                    switch (m_nit_guard) {
                        case 3/*1/4*/:
                            ret = 7.465;
                            break;
                        case 2/*1/8*/:
                            ret = 8.294;
                            break;
                        case 1/*1/16*/:
                            ret = 8.782;
                            break;
                        case 0/*1/32*/:
                            ret = 9.048;
                            break;
                    }
                    break;
                case 3/*5/6*/:
                    switch (m_nit_guard) {
                        case 3/*1/4*/:
                            ret = 8.294;
                            break;
                        case 2/*1/8*/:
                            ret = 9.216;
                            break;
                        case 1/*1/16*/:
                            ret = 9.758;
                            break;
                        case 0/*1/32*/:
                            ret = 10.053;
                            break;
                    }
                    break;
                case 4/*7/8*/:
                    switch (m_nit_guard) {
                        case 3/*1/4*/:
                            ret = 8.709;
                            break;
                        case 2/*1/8*/:
                            ret = 9.676;
                            break;
                        case 1/*1/16*/:
                            ret = 10.246;
                            break;
                        case 0/*1/32*/:
                            ret = 10.556;
                            break;
                    }
                    break;
            }
            break;
        case 1 /*16-QAM*/:
            switch (m_nit_code_rate) {
                case 0/*1/2*/:
                    switch (m_nit_guard) {
                        case 3/*1/4*/:
                            ret = 9.953;
                            break;
                        case 2/*1/8*/:
                            ret = 11.059;
                            break;
                        case 1/*1/16*/:
                            ret = 11.709;
                            break;
                        case 0/*1/32*/:
                            ret = 12.064;
                            break;
                    }
                    break;
                case 1/*2/3*/:
                    switch (m_nit_guard) {
                        case 3/*1/4*/:
                            ret = 13.271;
                            break;
                        case 2/*1/8*/:
                            ret = 14.745;
                            break;
                        case 1/*1/16*/:
                            ret = 15.612;
                            break;
                        case 0/*1/32*/:
                            ret = 16.086;
                            break;
                    }
                    break;
                case 2/*3/4*/:
                    switch (m_nit_guard) {
                        case 3/*1/4*/:
                            ret = 14.929;
                            break;
                        case 2/*1/8*/:
                            ret = 16.585;
                            break;
                        case 1/*1/16*/:
                            ret = 17.564;
                            break;
                        case 0/*1/32*/:
                            ret = 18.096;
                            break;
                    }
                    break;
                case 3/*5/6*/:
                    switch (m_nit_guard) {
                        case 3/*1/4*/:
                            ret = 16.588;
                            break;
                        case 2/*1/8*/:
                            ret = 18.431;
                            break;
                        case 1/*1/16*/:
                            ret = 19.516;
                            break;
                        case 0/*1/32*/:
                            ret = 20.107;
                            break;
                    }
                    break;
                case 4/*7/8*/:
                    switch (m_nit_guard) {
                        case 3/*1/4*/:
                            ret = 17.418;
                            break;
                        case 2/*1/8*/:
                            ret = 19.353;
                            break;
                        case 1/*1/16*/:
                            ret = 20.491;
                            break;
                        case 0/*1/32*/:
                            ret = 21.112;
                            break;
                    }
                    break;
            }
            break;
        case 2 /*64-QAM*/:
            switch (m_nit_code_rate) {
                case 0/*1/2*/:
                    switch (m_nit_guard) {
                        case 3/*1/4*/:
                            ret = 14.929;
                            break;
                        case 2/*1/8*/:
                            ret = 16.588;
                            break;
                        case 1/*1/16*/:
                            ret = 17.564;
                            break;
                        case 0/*1/32*/:
                            ret = 18.096;
                            break;
                    }
                    break;
                case 1/*2/3*/:
                    switch (m_nit_guard) {
                        case 3/*1/4*/:
                            ret = 19.906;
                            break;
                        case 2/*1/8*/:
                            ret = 22.118;
                            break;
                        case 1/*1/16*/:
                            ret = 23.419;
                            break;
                        case 0/*1/32*/:
                            ret = 24.128;
                            break;
                    }
                    break;
                case 2/*3/4*/:
                    switch (m_nit_guard) {
                        case 3/*1/4*/:
                            ret = 22.394;
                            break;
                        case 2/*1/8*/:
                            ret = 24.882;
                            break;
                        case 1/*1/16*/:
                            ret = 26.346;
                            break;
                        case 0/*1/32*/:
                            ret = 27.144;
                            break;
                    }
                    break;
                case 3/*5/6*/:
                    switch (m_nit_guard) {
                        case 3/*1/4*/:
                            ret = 24.882;
                            break;
                        case 2/*1/8*/:
                            ret = 27.647;
                            break;
                        case 1/*1/16*/:
                            ret = 29.273;
                            break;
                        case 0/*1/32*/:
                            ret = 30.160;
                            break;
                    }
                    break;
                case 4/*7/8*/:
                    switch (m_nit_guard) {
                        case 3/*1/4*/:
                            ret = 26.126;
                            break;
                        case 2/*1/8*/:
                            ret = 29.029;
                            break;
                        case 1/*1/16*/:
                            ret = 30.737;
                            break;
                        case 0/*1/32*/:
                            ret = 31.668;
                            break;
                    }
                    break;
            }
            break;
    }

    return ret;
}

#endif // TABLE_H_

