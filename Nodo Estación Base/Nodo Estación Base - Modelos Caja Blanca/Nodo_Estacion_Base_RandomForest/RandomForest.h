#pragma once
#include <cstdarg>
namespace Eloquent {
    namespace ML {
        namespace Port {
            class RandomForest {
                public:
                    /**
                    * Predict class for features vector
                    */
                    int predict(float *x) {
                        uint8_t votes[4] = { 0 };
                        // tree #1
                        if (x[0] <= 0.1707054078578949) {
                            if (x[2] <= 0.012338461354374886) {
                                votes[1] += 1;
                            }

                            else {
                                votes[2] += 1;
                            }
                        }

                        else {
                            if (x[0] <= 0.2553139179944992) {
                                votes[0] += 1;
                            }

                            else {
                                votes[3] += 1;
                            }
                        }

                        // tree #2
                        if (x[3] <= 0.07214399799704552) {
                            if (x[3] <= 0.02887200005352497) {
                                if (x[2] <= 0.012369230855256319) {
                                    votes[1] += 1;
                                }

                                else {
                                    votes[2] += 1;
                                }
                            }

                            else {
                                votes[0] += 1;
                            }
                        }

                        else {
                            votes[3] += 1;
                        }

                        // tree #3
                        if (x[1] <= 0.19644007086753845) {
                            if (x[3] <= 0.014400000218302011) {
                                votes[1] += 1;
                            }

                            else {
                                if (x[3] <= 0.02884799987077713) {
                                    votes[2] += 1;
                                }

                                else {
                                    votes[0] += 1;
                                }
                            }
                        }

                        else {
                            votes[3] += 1;
                        }

                        // tree #4
                        if (x[0] <= 0.1709149107336998) {
                            if (x[0] <= 0.08612021058797836) {
                                votes[1] += 1;
                            }

                            else {
                                votes[2] += 1;
                            }
                        }

                        else {
                            if (x[0] <= 0.2553089112043381) {
                                votes[0] += 1;
                            }

                            else {
                                votes[3] += 1;
                            }
                        }

                        // tree #5
                        if (x[3] <= 0.07188000157475471) {
                            if (x[3] <= 0.01435199985280633) {
                                votes[1] += 1;
                            }

                            else {
                                if (x[1] <= 0.14852465689182281) {
                                    votes[2] += 1;
                                }

                                else {
                                    votes[0] += 1;
                                }
                            }
                        }

                        else {
                            votes[3] += 1;
                        }

                        // tree #6
                        if (x[3] <= 0.07188000157475471) {
                            if (x[2] <= 0.012369230855256319) {
                                votes[1] += 1;
                            }

                            else {
                                if (x[1] <= 0.14857374131679535) {
                                    votes[2] += 1;
                                }

                                else {
                                    votes[0] += 1;
                                }
                            }
                        }

                        else {
                            votes[3] += 1;
                        }

                        // tree #7
                        if (x[3] <= 0.01435199985280633) {
                            votes[1] += 1;
                        }

                        else {
                            if (x[0] <= 0.1707054078578949) {
                                votes[2] += 1;
                            }

                            else {
                                if (x[1] <= 0.19644007086753845) {
                                    votes[0] += 1;
                                }

                                else {
                                    votes[3] += 1;
                                }
                            }
                        }

                        // tree #8
                        if (x[0] <= 0.25581058859825134) {
                            if (x[2] <= 0.012338461354374886) {
                                votes[1] += 1;
                            }

                            else {
                                if (x[2] <= 0.03698461502790451) {
                                    votes[2] += 1;
                                }

                                else {
                                    votes[0] += 1;
                                }
                            }
                        }

                        else {
                            votes[3] += 1;
                        }

                        // tree #9
                        if (x[3] <= 0.02887200005352497) {
                            if (x[0] <= 0.08623377233743668) {
                                votes[1] += 1;
                            }

                            else {
                                votes[2] += 1;
                            }
                        }

                        else {
                            if (x[0] <= 0.25581058859825134) {
                                votes[0] += 1;
                            }

                            else {
                                votes[3] += 1;
                            }
                        }

                        // tree #10
                        if (x[0] <= 0.2553089112043381) {
                            if (x[2] <= 0.012338461354374886) {
                                votes[1] += 1;
                            }

                            else {
                                if (x[1] <= 0.14852465689182281) {
                                    votes[2] += 1;
                                }

                                else {
                                    votes[0] += 1;
                                }
                            }
                        }

                        else {
                            votes[3] += 1;
                        }

                        // tree #11
                        if (x[0] <= 0.08623377233743668) {
                            votes[1] += 1;
                        }

                        else {
                            if (x[0] <= 0.17087146639823914) {
                                votes[2] += 1;
                            }

                            else {
                                if (x[0] <= 0.2553139179944992) {
                                    votes[0] += 1;
                                }

                                else {
                                    votes[3] += 1;
                                }
                            }
                        }

                        // tree #12
                        if (x[2] <= 0.09846153855323792) {
                            if (x[3] <= 0.014400000218302011) {
                                votes[1] += 1;
                            }

                            else {
                                if (x[2] <= 0.03698461502790451) {
                                    votes[2] += 1;
                                }

                                else {
                                    votes[0] += 1;
                                }
                            }
                        }

                        else {
                            votes[3] += 1;
                        }

                        // tree #13
                        if (x[3] <= 0.07216800004243851) {
                            if (x[1] <= 0.07644446566700935) {
                                votes[1] += 1;
                            }

                            else {
                                if (x[3] <= 0.02882399968802929) {
                                    votes[2] += 1;
                                }

                                else {
                                    votes[0] += 1;
                                }
                            }
                        }

                        else {
                            votes[3] += 1;
                        }

                        // tree #14
                        if (x[0] <= 0.17074885219335556) {
                            if (x[3] <= 0.014400000218302011) {
                                votes[1] += 1;
                            }

                            else {
                                votes[2] += 1;
                            }
                        }

                        else {
                            if (x[2] <= 0.09846153855323792) {
                                votes[0] += 1;
                            }

                            else {
                                votes[3] += 1;
                            }
                        }

                        // tree #15
                        if (x[2] <= 0.037107693031430244) {
                            if (x[3] <= 0.014400000218302011) {
                                votes[1] += 1;
                            }

                            else {
                                votes[2] += 1;
                            }
                        }

                        else {
                            if (x[3] <= 0.07195200026035309) {
                                votes[0] += 1;
                            }

                            else {
                                votes[3] += 1;
                            }
                        }

                        // tree #16
                        if (x[1] <= 0.19643035531044006) {
                            if (x[1] <= 0.14852465689182281) {
                                if (x[1] <= 0.07641646638512611) {
                                    votes[1] += 1;
                                }

                                else {
                                    votes[2] += 1;
                                }
                            }

                            else {
                                votes[0] += 1;
                            }
                        }

                        else {
                            votes[3] += 1;
                        }

                        // tree #17
                        if (x[1] <= 0.19662290066480637) {
                            if (x[0] <= 0.08611443266272545) {
                                votes[1] += 1;
                            }

                            else {
                                if (x[1] <= 0.14852465689182281) {
                                    votes[2] += 1;
                                }

                                else {
                                    votes[0] += 1;
                                }
                            }
                        }

                        else {
                            votes[3] += 1;
                        }

                        // tree #18
                        if (x[1] <= 0.07644446566700935) {
                            votes[1] += 1;
                        }

                        else {
                            if (x[3] <= 0.02884799987077713) {
                                votes[2] += 1;
                            }

                            else {
                                if (x[1] <= 0.19644007086753845) {
                                    votes[0] += 1;
                                }

                                else {
                                    votes[3] += 1;
                                }
                            }
                        }

                        // tree #19
                        if (x[1] <= 0.07644446566700935) {
                            votes[1] += 1;
                        }

                        else {
                            if (x[2] <= 0.03698461502790451) {
                                votes[2] += 1;
                            }

                            else {
                                if (x[0] <= 0.2553139179944992) {
                                    votes[0] += 1;
                                }

                                else {
                                    votes[3] += 1;
                                }
                            }
                        }

                        // tree #20
                        if (x[2] <= 0.012338461354374886) {
                            votes[1] += 1;
                        }

                        else {
                            if (x[2] <= 0.03723076917231083) {
                                votes[2] += 1;
                            }

                            else {
                                if (x[2] <= 0.09846153855323792) {
                                    votes[0] += 1;
                                }

                                else {
                                    votes[3] += 1;
                                }
                            }
                        }

                        // return argmax of votes
                        uint8_t classIdx = 0;
                        float maxVotes = votes[0];

                        for (uint8_t i = 1; i < 4; i++) {
                            if (votes[i] > maxVotes) {
                                classIdx = i;
                                maxVotes = votes[i];
                            }
                        }

                        return classIdx;
                    }

                protected:
                };
            }
        }
    }
