import random

def generate_orthogonal_floor(num_corners):
    # n must be even for an orthogonal closed path
    if num_corners % 2 != 0: num_corners -= 1
    
    steps = []
    # Generate n/2 horizontal and n/2 vertical moves
    # Simplified: generate segments and then ensure closure
    half = num_corners // 2
    
    # Verticals (N/S)
    v_moves = [random.randint(1, 20) for _ in range(half)]
    # Half are N, half are S (roughly) - to close, sum must be 0
    # Let's just do a simple approach:
    # First half-1 are random N, last is sum of others S
    # Actually, simpler: N, E, N, E ... then S, W to close.
    # Case n=4: N, E, S, W
    # Case n=6: N, E, N, E, S(sum), W(sum)
    
    segments = []
    curr_x, curr_y = 0, 0
    
    for i in range((num_corners - 2) // 2):
        # Vertical move
        dist_y = random.randint(1, 15)
        dir_y = 'N' if random.random() > 0.3 else 'S'
        segments.append((dir_y, dist_y))
        curr_y += dist_y if dir_y == 'N' else -dist_y
        
        # Horizontal move
        dist_x = random.randint(1, 15)
        dir_x = 'E' if random.random() > 0.3 else 'W'
        segments.append((dir_x, dist_x))
        curr_x += dist_x if dir_x == 'E' else -dist_x

    # Add closure moves
    if curr_y != 0:
        dir_y = 'S' if curr_y > 0 else 'N'
        segments.append((dir_y, abs(curr_y)))
    else:
        # If already at 0, add a dummy pair to keep corner count
        segments.append(('N', 5))
        segments.append(('S', 5))
        
    if curr_x != 0:
        dir_x = 'W' if curr_x > 0 else 'E'
        segments.append((dir_x, abs(curr_x)))
    else:
        segments.append(('E', 5))
        segments.append(('W', 5))

    # Adjust n to match segments
    n = len(segments)
    
    # Format: (1x, I2, 2X, 12(A1, I2, 2X))
    card = f" {n:02d}  "
    for d, dist in segments:
        card += f"{d}{dist:02d}  "
    return card

print("=== GENERATING 10 RANDOM FLOORS ===")
for i in range(10):
    corners = random.choice([4, 6, 8, 10, 12])
    print(generate_orthogonal_floor(corners))
