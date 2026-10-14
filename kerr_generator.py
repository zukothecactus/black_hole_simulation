import sympy as sp
import re # for cleaning up the GLSL code

# 1. Define real symbolic variables
t, x, y, z, M, a = sp.symbols('pos_t pos_x pos_y pos_z M a', real=True)
coords = [t, x, y, z]

vt, vx, vy, vz = sp.symbols('vt vx vy vz', real=True)
velocities = [vt, vx, vy, vz]

print("Building Kerr-Schild Metric...")
R2 = x**2 + y**2 + z**2
a2 = a**2

# Explicit solution for Kerr radius r^2
r2 = 0.5 * (R2 - a2 + sp.sqrt((R2 - a2)**2 + 4 * a2 * z**2))
r = sp.sqrt(r2)

# Scalar potential f
f = (2.0 * M * r**3) / (r**4 + a2 * z**2)

# Null vector l_mu (lower indices)
denom = r2 + a2
l_lower = [1.0, (r * x + a * y) / denom, (r * y - a * x) / denom, z / r]

# Minkowski metric (eta)
eta = sp.Matrix([[-1,0,0,0],[0,1,0,0],[0,0,1,0],[0,0,0,1]])

# Construct g_mu_nu = eta_mu_nu + f * l_mu * l_nu
g_mu_nu = sp.zeros(4, 4)
for i in range(4):
    for j in range(4):
        g_mu_nu[i, j] = eta[i, j] + f * l_lower[i] * l_lower[j]

# Analytical Inverse to prevent SymPy from freezing
l_upper = [-1.0, l_lower[1], l_lower[2], l_lower[3]]
g_inv = sp.zeros(4, 4)
for i in range(4):
    for j in range(4):
        g_inv[i, j] = eta[i, j] - f * l_upper[i] * l_upper[j]

print("Calculating Christoffel Symbols and Acceleration (Takes ~10-30 seconds)...")
accel = [0, 0, 0, 0]
for mu in range(4):
    for alpha in range(4):
        for beta in range(4):
            christoffel = 0
            for rho in range(4):
                term1 = sp.diff(g_mu_nu[beta, rho], coords[alpha])
                term2 = sp.diff(g_mu_nu[alpha, rho], coords[beta])
                term3 = sp.diff(g_mu_nu[alpha, beta], coords[rho])
                christoffel += 0.5 * g_inv[mu, rho] * (term1 + term2 - term3)
            
            accel[mu] -= christoffel * velocities[alpha] * velocities[beta]

print("Optimizing equations via Common Subexpression Elimination...")
# Extract repeated calculations into temporary variables
optimizations, final_accel = sp.cse(accel)

print("\n--- COPY BELOW THIS LINE INTO YOUR GLSL SHADER ---")

def clean_glsl(expr):
    # Convert SymPy expression to C code string
    code = sp.ccode(expr)
    
    # 1. Normalize fractional representations
    code = code.replace("3.0/2.0", "1.5").replace("5.0/2.0", "2.5")
    code = code.replace(" 1/2", " 0.5").replace("(1/2)", "(0.5)")
    
    # 2. Fast Math: Eliminate pow() for squares and common fractions
    code = re.sub(r'pow\(([^,]+),\s*2(?:\.0)?\)', r'((\1) * (\1))', code)
    code = re.sub(r'pow\(([^,]+),\s*-2(?:\.0)?\)', r'(1.0 / ((\1) * (\1)))', code)
    code = re.sub(r'pow\(([^,]+),\s*3(?:\.0)?\)', r'((\1) * (\1) * (\1))', code)
    
    # Matches pow(var, 1.5) -> var * sqrt(var)
    code = re.sub(r'pow\(([^,]+),\s*1\.5\)', r'((\1) * sqrt(\1))', code)
    # Matches pow(var, 2.5) -> var * var * sqrt(var)
    code = re.sub(r'pow\(([^,]+),\s*2\.5\)', r'((\1) * (\1) * sqrt(\1))', code)
    
    # 3. Strip redundant 1.0 * multipliers (negative lookbehind prevents changing 21.0*x to 2x)
    code = re.sub(r'(?<!\d)1\.0\*', '', code)
    
    # 4. Fix strict GLSL float typing for any remaining pow() functions
    for i in range(2, 10):
        code = code.replace(f", {i})", f", {i}.0)")
        code = code.replace(f", -{i})", f", -{i}.0)")
        
    return code

for var_name, expr in optimizations:
    print(f"float {var_name} = {clean_glsl(expr)};")

print()
for i in range(4):
    print(f"acc[{i}] = {clean_glsl(final_accel[i])};")