import java.util.Arrays;
import java.util.Scanner;

public class Solution {
    private static final int MOD = 1000000007;

    // fast exponentiation mod
    private static long modpow(long a, long e) {
        long r = 1;
        while (e > 0) {
            if ((e & 1) == 1) r = (r * a) % MOD;
            a = (a * a) % MOD;
            e >>= 1;
        }
        return r;
    }

    public static void main(String[] args) {
        Scanner scanner = new Scanner(System.in);
        int N = scanner.nextInt();
        int A = scanner.nextInt();
        int B = scanner.nextInt();
        scanner.close();

        int L = A + B;
        int D = N - L;

        // Precompute factorials up to N:
        long[] fact = new long[N + 1];
        long[] invf = new long[N + 1];
        fact[0] = 1;
        for (int i = 1; i <= N; i++) fact[i] = (fact[i - 1] * i) % MOD;
        invf[N] = modpow(fact[N], MOD - 2);
        for (int i = N; i > 0; i--) invf[i - 1] = (invf[i] * i) % MOD;

        // Function C(n,k)
        java.util.function.BiFunction<Integer, Integer, Long> C = (n, k) -> {
            if (k < 0 || k > n) return 0L;
            return (((fact[n] * invf[k]) % MOD) * invf[n - k]) % MOD;
        };

        // G[k] = # of win‐only sequences S of length L
        //         with A As, B Bs, prefix As>=Bs,
        //         and exactly k "forced gaps" (i.e. adjacent equal wins in S).
        // We'll build G by DP over (iA,iB,last), rolling over iA+iB = len so far,
        // and maintaining a counter of how many forced gaps we've used.
        long[] G = new long[L + 1];
        Arrays.fill(G, 0);

        // dpA[b][k] = # ways with (a=i, b=b), last win=A, used k forced gaps
        // dpB[b][k] = same but last=B.
        // We only store current and next a-layer, and only b=0..B, k=0..L-1.
        // In practice we fold k-index into a single vector per last-letter state.
        long[][] dpA = new long[B + 1][L + 1];
        long[][] dpB = new long[B + 1][L + 1];
        long[][] ndpA = new long[B + 1][L + 1];
        long[][] ndpB = new long[B + 1][L + 1];

        // base: 1 A-win starting sequence (iA=1,iB=0,last=A,no forced gap).
        if (A > 0) dpA[0][0] = 1;

        for (int a = 1; a <= A; a++) {
            // Clear next layer
            for (int b = 0; b <= B; b++) {
                Arrays.fill(ndpA[b], 0);
                Arrays.fill(ndpB[b], 0);
            }
            for (int b = 0; b <= Math.min(B, a); b++) {
                for (int k = 0; k <= L; k++) {
                    long vA = dpA[b][k];
                    long vB = dpB[b][k];
                    if (vA == 0 && vB == 0) continue;
                    // 1) Append an A-win:
                    //    from dpA: forbidden unless we insert a forced gap
                    //    but in S (no draws) every time we do AA we pay one forced gap.
                    if (a > 0) {
                        // from dpA => new forced gap
                        if (vA > 0) {
                            if (k + 1 <= L) {
                                ndpA[b][k + 1] = (ndpA[b][k + 1] + vA) % MOD;
                            }
                        }
                        // from dpB => no new gap
                        if (vB > 0) {
                            ndpA[b][k] = (ndpA[b][k] + vB) % MOD;
                        }
                    }
                    // 2) Append a B-win:
                    //    only if b+1 <= a-1 => prefix A>=B holds
                    if (b + 1 <= a - 1) {
                        // from dpB => new forced gap
                        if (vB > 0) {
                            if (k + 1 <= L) {
                                ndpB[b + 1][k + 1] = (ndpB[b + 1][k + 1] + vB) % MOD;
                            }
                        }
                        // from dpA => no new gap
                        if (vA > 0) {
                            ndpB[b + 1][k] = (ndpB[b + 1][k] + vA) % MOD;
                        }
                    }
                }
            }
            // swap into dp*
            long[][] tempA = dpA;
            dpA = ndpA;
            ndpA = tempA;
            long[][] tempB = dpB;
            dpB = ndpB;
            ndpB = tempB;
        }

        // collect G[k] from final dpA[B][k] + dpB[B][k]
        for (int k = 0; k <= L; k++) {
            if (A >= B) {
                G[k] = (dpA[B][k] + dpB[B][k]) % MOD;
            }
        }

        // Finally sum over k:
        // answer = Σ_{k=0..L} G[k] * C(N - 2*k, L - k)
        long ans = 0;
        for (int k = 0; k <= L; k++) {
            if (G[k] == 0) continue;
            int n2 = N - 2 * k;
            int r2 = L - k;
            if (n2 < 0 || r2 < 0 || r2 > n2) continue;
            long ways = C.apply(n2, r2);
            ans = (ans + ways * G[k]) % MOD;
        }

        System.out.println(ans);
    }
}