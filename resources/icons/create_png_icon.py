#!/usr/bin/env python3
"""
Create a simple WiFi icon PNG for system tray
"""
try:
    from PIL import Image, ImageDraw
    
    # Create a 22x22 icon (common tray icon size)
    sizes = [16, 22, 24, 32, 48]
    
    for size in sizes:
        img = Image.new('RGBA', (size, size), (0, 0, 0, 0))
        draw = ImageDraw.Draw(img)
        
        # Colors
        blue = (33, 150, 243, 255)  # #2196F3
        dark_blue = (25, 118, 210, 255)  # #1976D2
        
        # Scale factors
        scale = size / 22.0
        
        # Draw WiFi arcs (3 arcs)
        center_x = size // 2
        center_y = int(size * 0.7)
        
        # Outer arc
        arc_width = max(1, int(2 * scale))
        draw.arc([int(center_x - 10*scale), int(center_y - 10*scale), 
                  int(center_x + 10*scale), int(center_y + 10*scale)], 
                 180, 360, fill=blue, width=arc_width)
        
        # Middle arc
        draw.arc([int(center_x - 7*scale), int(center_y - 7*scale), 
                  int(center_x + 7*scale), int(center_y + 7*scale)], 
                 180, 360, fill=blue, width=arc_width)
        
        # Inner arc
        draw.arc([int(center_x - 4*scale), int(center_y - 4*scale), 
                  int(center_x + 4*scale), int(center_y + 4*scale)], 
                 180, 360, fill=blue, width=arc_width)
        
        # Center dot
        dot_size = max(2, int(3 * scale))
        draw.ellipse([center_x - dot_size//2, center_y - dot_size//2,
                      center_x + dot_size//2, center_y + dot_size//2], 
                     fill=dark_blue)
        
        # Save
        filename = f'wifi_{size}x{size}.png'
        img.save(filename)
        print(f'Created {filename}')
    
    # Also create the main wifi.png (22x22)
    img = Image.new('RGBA', (22, 22), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    
    blue = (33, 150, 243, 255)
    dark_blue = (25, 118, 210, 255)
    
    center_x = 11
    center_y = 15
    
    # Draw WiFi arcs
    draw.arc([1, 5, 21, 25], 180, 360, fill=blue, width=2)
    draw.arc([4, 8, 18, 22], 180, 360, fill=blue, width=2)
    draw.arc([7, 11, 15, 19], 180, 360, fill=blue, width=2)
    
    # Center dot
    draw.ellipse([10, 14, 13, 17], fill=dark_blue)

    img.save('wifi.png')
    print('Created wifi.png')
    
except ImportError:
    print("PIL/Pillow not installed. Skipping PNG creation.")
    print("Install with: pip install Pillow")

