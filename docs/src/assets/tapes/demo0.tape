# Output file for the GIF
Output cortex_text_generation.gif

# Set up terminal aesthetics
Set FontSize 30
Set Width 1800
Set Height 900
Set Theme "Catppuccin Frappe"

# Pull the model from Cortex
Type "cortex pull deepseek-r1-distill-qwen-1.5b:1.5b"
Enter
Wait

# Start the model in detached mode
Type "cortex run deepseek-r1-distill-qwen-1.5b:1.5b"
Enter
#Show
Wait

Type "Hi 👋"
Enter
Show
#Wait@30s
Sleep 10s

Type@50ms "Thank you for coming to my Ted Talk! 👋"
Sleep 3s
Backspace 39
Sleep 10s


# Exit Python shell
Type "exit()"
Enter
Sleep 1s

