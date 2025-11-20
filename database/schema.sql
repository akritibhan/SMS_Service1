-- Create database
CREATE DATABASE IF NOT EXISTS sms_service 
CHARACTER SET utf8mb4  
-- supports 4 bytes
COLLATE utf8mb4_unicode_ci;
-- how to sort and compare text, ci--> case insentive

USE sms_service;

-- Create SMS patterns table
CREATE TABLE IF NOT EXISTS sms_patterns (
    id INT AUTO_INCREMENT PRIMARY KEY,
    Sender VARCHAR(255) NOT NULL,
    SMS_Content TEXT NOT NULL,
    Entity_ID VARCHAR(100) NOT NULL,
    Content_ID VARCHAR(100) NOT NULL,
    TM_ID VARCHAR(100) NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    INDEX idx_sender (Sender),
    INDEX idx_entity (Entity_ID),
    INDEX idx_content (Content_ID)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
-- how mysql physically stores data on disk , acid ,trans support,fk constraun
-- Insert sample data for testing
-- Sender: BANK001 with various transaction patterns

INSERT INTO sms_patterns (Sender, SMS_Content, Entity_ID, Content_ID, TM_ID) VALUES
-- Account balance patterns
('BANK001', 'Your account balance is Rs\\.? ?[0-9,]+', 'ENT001', 'CNT001', 'TM001'),
('BANK001', 'Available balance: Rs\\.? ?[0-9,]+', 'ENT001', 'CNT002', 'TM002'),

-- Transaction patterns
('BANK001', 'Rs\\.? ?[0-9,]+ debited from.*account.*[0-9]+', 'ENT001', 'CNT003', 'TM003'),
('BANK001', 'Rs\\.? ?[0-9,]+ credited to.*account.*[0-9]+', 'ENT001', 'CNT004', 'TM004'),
('BANK001', 'Transaction of Rs\\.? ?[0-9,]+ was successful', 'ENT001', 'CNT005', 'TM005'),

-- ATM withdrawal patterns
('BANK001', 'ATM Withdrawal.*Rs\\.? ?[0-9,]+.*at.*', 'ENT001', 'CNT006', 'TM006'),
('BANK001', 'Cash withdrawn.*Rs\\.? ?[0-9,]+.*', 'ENT001', 'CNT007', 'TM007'),

-- Payment patterns
('BANK001', 'Payment of Rs\\.? ?[0-9,]+ to.*successful', 'ENT001', 'CNT008', 'TM008'),
('BANK001', 'Bill payment.*Rs\\.? ?[0-9,]+.*completed', 'ENT001', 'CNT009', 'TM009'),

-- OTP patterns
('BANK001', 'Your OTP is [0-9]{4,6}', 'ENT001', 'CNT010', 'TM010'),
('BANK001', 'OTP for transaction is [0-9]{4,6}', 'ENT001', 'CNT011', 'TM011');

-- Sender: BANK002 with additional banking patterns
INSERT INTO sms_patterns (Sender, SMS_Content, Entity_ID, Content_ID, TM_ID) VALUES
('BANK002', 'Dear customer.*balance.*Rs\\.? ?[0-9,]+', 'ENT005', 'CNT035', 'TM035'),
('BANK002', 'Account.*debited.*Rs\\.? ?[0-9,]+.*on [0-9]{2}-[A-Za-z]{3}', 'ENT005', 'CNT036', 'TM036'),
('BANK002', 'Account.*credited.*Rs\\.? ?[0-9,]+.*on [0-9]{2}-[A-Za-z]{3}', 'ENT005', 'CNT037', 'TM037'),
('BANK002', 'IMPS.*Rs\\.? ?[0-9,]+.*transferred successfully', 'ENT005', 'CNT038', 'TM038'),
('BANK002', 'NEFT.*Rs\\.? ?[0-9,]+.*credited', 'ENT005', 'CNT039', 'TM039'),
('BANK002', 'RTGS.*Rs\\.? ?[0-9,]+.*completed', 'ENT005', 'CNT040', 'TM040'),
('BANK002', 'Loan EMI.*Rs\\.? ?[0-9,]+.*debited', 'ENT005', 'CNT041', 'TM041'),
('BANK002', 'Credit card.*Rs\\.? ?[0-9,]+.*paid', 'ENT005', 'CNT042', 'TM042'),
('BANK002', 'Mobile banking login.*OTP [0-9]{6}', 'ENT005', 'CNT043', 'TM043');

-- Sender: ECOMM01 with e-commerce patterns
INSERT INTO sms_patterns (Sender, SMS_Content, Entity_ID, Content_ID, TM_ID) VALUES
('ECOMM01', 'Order #[A-Z0-9]+ confirmed', 'ENT002', 'CNT012', 'TM012'),
('ECOMM01', 'Your order.*has been shipped', 'ENT002', 'CNT013', 'TM013'),
('ECOMM01', 'Delivery.*today between.*', 'ENT002', 'CNT014', 'TM014'),
('ECOMM01', 'Thank you for shopping.*Rs\\.? ?[0-9,]+', 'ENT002', 'CNT015', 'TM015'),
('ECOMM01', 'Your refund of Rs\\.? ?[0-9,]+ has been processed', 'ENT002', 'CNT016', 'TM016'),
('ECOMM01', 'Order.*out for delivery.*tracking [A-Z0-9]+', 'ENT002', 'CNT044', 'TM044'),
('ECOMM01', 'Package delivered.*Thank you', 'ENT002', 'CNT045', 'TM045'),
('ECOMM01', 'Return request.*approved.*refund', 'ENT002', 'CNT046', 'TM046'),
('ECOMM01', 'Cashback of Rs\\.? ?[0-9,]+.*credited to wallet', 'ENT002', 'CNT047', 'TM047'),
('ECOMM01', 'Order cancelled.*refund.*Rs\\.? ?[0-9,]+', 'ENT002', 'CNT048', 'TM048'),
('ECOMM01', 'Flash sale.*[0-9]+% off.*limited time', 'ENT002', 'CNT049', 'TM049'),
('ECOMM01', 'Wishlist item.*now available', 'ENT002', 'CNT050', 'TM050');

-- Sender: TRAVEL1 with booking patterns
INSERT INTO sms_patterns (Sender, SMS_Content, Entity_ID, Content_ID, TM_ID) VALUES
('TRAVEL1', 'Flight booking confirmed.*PNR.*', 'ENT003', 'CNT017', 'TM017'),
('TRAVEL1', 'Train ticket.*PNR [A-Z0-9]{10}', 'ENT003', 'CNT018', 'TM018'),
('TRAVEL1', 'Hotel booking.*confirmation.*', 'ENT003', 'CNT019', 'TM019'),
('TRAVEL1', 'Booking cancelled.*refund.*', 'ENT003', 'CNT020', 'TM020'),
('TRAVEL1', 'Web check-in.*flight.*[A-Z]{2}[0-9]{3,4}', 'ENT003', 'CNT051', 'TM051'),
('TRAVEL1', 'Bus ticket.*booked.*seat [0-9A-Z]+', 'ENT003', 'CNT052', 'TM052'),
('TRAVEL1', 'Cab booked.*driver.*[0-9]{10}', 'ENT003', 'CNT053', 'TM053');

-- Sender: TELCO with telecom patterns
INSERT INTO sms_patterns (Sender, SMS_Content, Entity_ID, Content_ID, TM_ID) VALUES
('TELCO', 'Your recharge of Rs\\.? ?[0-9,]+ is successful', 'ENT004', 'CNT021', 'TM021'),
('TELCO', 'Data balance.*GB remaining', 'ENT004', 'CNT022', 'TM022'),
('TELCO', 'Your bill of Rs\\.? ?[0-9,]+ is due', 'ENT004', 'CNT023', 'TM023'),
('TELCO', 'Validity extended till.*', 'ENT004', 'CNT024', 'TM024'),
('TELCO', 'Talk time balance.*Rs\\.? ?[0-9.]+', 'ENT004', 'CNT054', 'TM054'),
('TELCO', 'Data pack.*activated.*validity [0-9]+ days', 'ENT004', 'CNT055', 'TM055'),
('TELCO', 'Bill payment.*Rs\\.? ?[0-9,]+.*received', 'ENT004', 'CNT056', 'TM056');

-- Sender: HEALTH with healthcare patterns
INSERT INTO sms_patterns (Sender, SMS_Content, Entity_ID, Content_ID, TM_ID) VALUES
('HEALTH', 'Appointment confirmed.*Dr\\..*on [0-9]{2}-[A-Za-z]{3}', 'ENT006', 'CNT057', 'TM057'),
('HEALTH', 'Lab report ready.*download.*', 'ENT006', 'CNT058', 'TM058'),
('HEALTH', 'Prescription.*uploaded.*view online', 'ENT006', 'CNT059', 'TM059'),
('HEALTH', 'Medicine order.*out for delivery', 'ENT006', 'CNT060', 'TM060'),
('HEALTH', 'Health checkup.*Rs\\.? ?[0-9,]+.*booked', 'ENT006', 'CNT061', 'TM061'),
('HEALTH', 'Vaccination reminder.*[A-Za-z]+.*due', 'ENT006', 'CNT062', 'TM062');

-- Sender: FOOD with food delivery patterns
INSERT INTO sms_patterns (Sender, SMS_Content, Entity_ID, Content_ID, TM_ID) VALUES
('FOOD', 'Order #[0-9]+.*confirmed.*Rs\\.? ?[0-9,]+', 'ENT007', 'CNT063', 'TM063'),
('FOOD', 'Your order.*being prepared', 'ENT007', 'CNT064', 'TM064'),
('FOOD', 'Delivery partner.*picked up.*order', 'ENT007', 'CNT065', 'TM065'),
('FOOD', 'Order delivered.*Enjoy your meal', 'ENT007', 'CNT066', 'TM066'),
('FOOD', 'Refund.*Rs\\.? ?[0-9,]+.*processed', 'ENT007', 'CNT067', 'TM067'),
('FOOD', 'Coupon.*[A-Z0-9]+.*get [0-9]+% off', 'ENT007', 'CNT068', 'TM068');

-- Sender: INSURE with insurance patterns
INSERT INTO sms_patterns (Sender, SMS_Content, Entity_ID, Content_ID, TM_ID) VALUES
('INSURE', 'Policy.*[A-Z0-9]+.*premium.*Rs\\.? ?[0-9,]+.*due', 'ENT008', 'CNT069', 'TM069'),
('INSURE', 'Premium payment.*Rs\\.? ?[0-9,]+.*received', 'ENT008', 'CNT070', 'TM070'),
('INSURE', 'Policy renewed.*valid till.*', 'ENT008', 'CNT071', 'TM071'),
('INSURE', 'Claim.*initiated.*reference [A-Z0-9]+', 'ENT008', 'CNT072', 'TM072'),
('INSURE', 'Claim approved.*Rs\\.? ?[0-9,]+.*credited', 'ENT008', 'CNT073', 'TM073');

-- Sender: EDUCAT with education patterns
INSERT INTO sms_patterns (Sender, SMS_Content, Entity_ID, Content_ID, TM_ID) VALUES
('EDUCAT', 'Fee payment.*Rs\\.? ?[0-9,]+.*received', 'ENT009', 'CNT074', 'TM074'),
('EDUCAT', 'Class.*scheduled.*[0-9]{2}:[0-9]{2}', 'ENT009', 'CNT075', 'TM075'),
('EDUCAT', 'Exam result.*published.*check portal', 'ENT009', 'CNT076', 'TM076'),
('EDUCAT', 'Admission confirmed.*roll number [A-Z0-9]+', 'ENT009', 'CNT077', 'TM077'),
('EDUCAT', 'Assignment.*submitted successfully', 'ENT009', 'CNT078', 'TM078');

-- Sender: GOVT with government service patterns
INSERT INTO sms_patterns (Sender, SMS_Content, Entity_ID, Content_ID, TM_ID) VALUES
('GOVT', 'Application.*[A-Z0-9]+.*submitted', 'ENT010', 'CNT079', 'TM079'),
('GOVT', 'Document.*verified.*status updated', 'ENT010', 'CNT080', 'TM080'),
('GOVT', 'Service request.*approved.*ref [A-Z0-9]+', 'ENT010', 'CNT081', 'TM081'),
('GOVT', 'Tax payment.*Rs\\.? ?[0-9,]+.*acknowledgment [A-Z0-9]+', 'ENT010', 'CNT082', 'TM082'),
('GOVT', 'Certificate.*ready.*collect from.*', 'ENT010', 'CNT083', 'TM083');

-- Add more patterns for BANK001 to simulate high-volume sender
INSERT INTO sms_patterns (Sender, SMS_Content, Entity_ID, Content_ID, TM_ID) VALUES
('BANK001', 'Minimum due amount is Rs\\.? ?[0-9,]+', 'ENT001', 'CNT025', 'TM025'),
('BANK001', 'Credit card statement.*generated', 'ENT001', 'CNT026', 'TM026'),
('BANK001', 'EMI of Rs\\.? ?[0-9,]+ has been deducted', 'ENT001', 'CNT027', 'TM027'),
('BANK001', 'Fund transfer.*Rs\\.? ?[0-9,]+.*successful', 'ENT001', 'CNT028', 'TM028'),
('BANK001', 'UPI payment.*Rs\\.? ?[0-9,]+.*to.*', 'ENT001', 'CNT029', 'TM029'),
('BANK001', 'Auto-debit.*Rs\\.? ?[0-9,]+.*successful', 'ENT001', 'CNT030', 'TM030'),
('BANK001', 'Standing instruction.*executed', 'ENT001', 'CNT031', 'TM031'),
('BANK001', 'Cheque number [0-9]+ cleared', 'ENT001', 'CNT032', 'TM032'),
('BANK001', 'Interest credited.*Rs\\.? ?[0-9,]+', 'ENT001', 'CNT033', 'TM033'),
('BANK001', 'Fixed deposit.*matured', 'ENT001', 'CNT034', 'TM034');

-- Verify data
SELECT 
    Sender, 
    COUNT(*) as pattern_count,
    MIN(Entity_ID) as entity_id
FROM sms_patterns 
GROUP BY Sender
ORDER BY pattern_count DESC;

SELECT COUNT(*) as total_patterns FROM sms_patterns;

-- Show distribution statistics
SELECT 
    'Total Patterns' as metric,
    COUNT(*) as count
FROM sms_patterns
UNION ALL
SELECT 
    'Total Senders' as metric,
    COUNT(DISTINCT Sender) as count
FROM sms_patterns
UNION ALL
SELECT 
    'Total Entities' as metric,
    COUNT(DISTINCT Entity_ID) as count
FROM sms_patterns;
