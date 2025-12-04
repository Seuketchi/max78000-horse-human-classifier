#!/usr/bin/env python3
"""
MAX78000 Image Capture Script

This script captures images streamed from the MAX78000 over serial
and saves them as image files along with classification results.

Usage:
    python capture_images.py --port COM3
    python capture_images.py --port /dev/ttyUSB0 --baud 115200

Requirements:
    pip install pyserial pillow
"""

import serial
import argparse
import os
import re
import sys
from datetime import datetime
from pathlib import Path

try:
    from PIL import Image
except ImportError:
    print("Please install Pillow: pip install pillow")
    sys.exit(1)


class ImageCapture:
    def __init__(self, port, baud=115200, output_dir="captures"):
        self.port = port
        self.baud = baud
        self.output_dir = Path(output_dir)
        self.output_dir.mkdir(exist_ok=True)
        self.serial = None
        self.capture_count = 0
        
    def connect(self):
        """Connect to serial port."""
        try:
            self.serial = serial.Serial(self.port, self.baud, timeout=1)
            print(f"Connected to {self.port} at {self.baud} baud")
            return True
        except serial.SerialException as e:
            print(f"Error connecting to {self.port}: {e}")
            return False
    
    def disconnect(self):
        """Disconnect from serial port."""
        if self.serial:
            self.serial.close()
            print("Disconnected")
    
    def read_line(self):
        """Read a line from serial."""
        try:
            line = self.serial.readline().decode('utf-8', errors='ignore').strip()
            return line
        except:
            return None
    
    def parse_result(self, lines):
        """Parse classification result from lines."""
        result = {}
        for line in lines:
            if line.startswith("CAPTURE_ID:"):
                result['capture_id'] = int(line.split(":")[1])
            elif line.startswith("CLASS:"):
                result['class'] = line.split(":")[1]
            elif line.startswith("CONFIDENCE:"):
                result['confidence'] = int(line.split(":")[1])
            elif line.startswith("INFERENCE_TIME_US:"):
                result['inference_time_us'] = int(line.split(":")[1])
        return result
    
    def parse_ppm(self, lines, width, height):
        """Parse PPM image data from lines."""
        pixels = []
        
        for line in lines:
            # Skip PPM header lines
            if line.startswith("P3") or line.startswith("#") or line == "255":
                continue
            if line == f"{width} {height}":
                continue
                
            # Parse RGB values
            values = line.split()
            for v in values:
                try:
                    pixels.append(int(v))
                except ValueError:
                    pass
        
        return pixels
    
    def create_image(self, pixels, width, height):
        """Create PIL Image from pixel data."""
        if len(pixels) != width * height * 3:
            print(f"Warning: Expected {width*height*3} values, got {len(pixels)}")
            return None
        
        img = Image.new('RGB', (width, height))
        pixel_data = []
        
        for i in range(0, len(pixels), 3):
            r = pixels[i]
            g = pixels[i + 1]
            b = pixels[i + 2]
            pixel_data.append((r, g, b))
        
        img.putdata(pixel_data)
        return img
    
    def save_capture(self, img, result):
        """Save image and result to files."""
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        capture_id = result.get('capture_id', self.capture_count)
        class_name = result.get('class', 'unknown')
        confidence = result.get('confidence', 0)
        
        # Create filename
        filename = f"capture_{capture_id:04d}_{class_name}_{confidence}pct_{timestamp}"
        
        # Save image
        img_path = self.output_dir / f"{filename}.png"
        img.save(img_path)
        print(f"Saved image: {img_path}")
        
        # Save result as text
        txt_path = self.output_dir / f"{filename}.txt"
        with open(txt_path, 'w') as f:
            f.write(f"Capture ID: {capture_id}\n")
            f.write(f"Class: {class_name}\n")
            f.write(f"Confidence: {confidence}%\n")
            f.write(f"Inference Time: {result.get('inference_time_us', 0)} us\n")
            f.write(f"Timestamp: {timestamp}\n")
        print(f"Saved result: {txt_path}")
        
        return img_path
    
    def run(self):
        """Main capture loop."""
        if not self.connect():
            return
        
        print("\n" + "="*50)
        print("MAX78000 Image Capture")
        print("="*50)
        print("Waiting for captures from device...")
        print("Press Ctrl+C to exit\n")
        
        current_result = {}
        image_lines = []
        capturing_image = False
        width = 128
        height = 128
        
        try:
            while True:
                line = self.read_line()
                if not line:
                    continue
                
                # Check for result marker
                if "<<<RESULT>>>" in line:
                    # Collect result lines
                    result_lines = []
                    while True:
                        line = self.read_line()
                        if not line or "<<<RESULT>>>" in line:
                            break
                        result_lines.append(line)
                    current_result = self.parse_result(result_lines)
                    print(f"\n[Result] Class: {current_result.get('class', '?')} "
                          f"({current_result.get('confidence', 0)}%)")
                
                # Check for image start
                elif "<<<IMG_START>>>" in line:
                    image_lines = []
                    capturing_image = False
                    print("[Receiving image...]")
                
                elif line.startswith("WIDTH:"):
                    width = int(line.split(":")[1])
                
                elif line.startswith("HEIGHT:"):
                    height = int(line.split(":")[1])
                
                elif line == "DATA_START":
                    capturing_image = True
                    image_lines = []
                
                elif line == "DATA_END":
                    capturing_image = False
                
                elif "<<<IMG_END>>>" in line:
                    # Process the captured image
                    if image_lines:
                        print(f"[Parsing {width}x{height} image...]")
                        pixels = self.parse_ppm(image_lines, width, height)
                        
                        if pixels:
                            img = self.create_image(pixels, width, height)
                            if img:
                                self.capture_count += 1
                                self.save_capture(img, current_result)
                        else:
                            print("Error: No pixel data parsed")
                    
                    image_lines = []
                    current_result = {}
                
                elif capturing_image:
                    image_lines.append(line)
                
                else:
                    # Print other output from device
                    if line and not line.startswith("P3"):
                        print(f"[Device] {line}")
        
        except KeyboardInterrupt:
            print("\n\nCapture stopped by user")
        
        finally:
            self.disconnect()
            print(f"\nTotal captures saved: {self.capture_count}")
            print(f"Output directory: {self.output_dir.absolute()}")


def main():
    parser = argparse.ArgumentParser(description="Capture images from MAX78000")
    parser.add_argument("--port", "-p", required=True, help="Serial port (e.g., COM3, /dev/ttyUSB0)")
    parser.add_argument("--baud", "-b", type=int, default=115200, help="Baud rate (default: 115200)")
    parser.add_argument("--output", "-o", default="captures", help="Output directory (default: captures)")
    
    args = parser.parse_args()
    
    capturer = ImageCapture(args.port, args.baud, args.output)
    capturer.run()


if __name__ == "__main__":
    main()
