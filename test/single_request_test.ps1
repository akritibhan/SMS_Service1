# Single request test to show sample request/response from different senders
Write-Host "Single Request Test - Multiple Senders" -ForegroundColor Cyan

# Array of sample requests from different senders
$sampleRequests = @(
    @{
        Name = "BANK001 - Debit Transaction"
        Body = @"
<?xml version="1.0" encoding="UTF-8"?>
<SMSRequest>
    <Sender>BANK001</Sender>
    <Receiver>9876543210</Receiver>
    <SMSContent>Rs. 5000 debited from account ending 1234</SMSContent>
</SMSRequest>
"@
    },
    @{
        Name = "HEALTH - Appointment"
        Body = @"
<?xml version="1.0" encoding="UTF-8"?>
<SMSRequest>
    <Sender>HEALTH</Sender>
    <Receiver>9876543210</Receiver>
    <SMSContent>Appointment confirmed with Dr. Smith on 20-Nov</SMSContent>
</SMSRequest>
"@
    },
    @{
        Name = "FOOD - Order Confirmation"
        Body = @"
<?xml version="1.0" encoding="UTF-8"?>
<SMSRequest>
    <Sender>FOOD</Sender>
    <Receiver>9876543210</Receiver>
    <SMSContent>Order #12345 confirmed Rs. 450</SMSContent>
</SMSRequest>
"@
    }
)

foreach ($sample in $sampleRequests) {
    Write-Host "Testing: $($sample.Name)" -ForegroundColor Yellow
    
    $body = $sample.Body

    Write-Host "REQUEST:" -ForegroundColor Yellow
    Write-Host $body
    Write-Host ""

    try {
        $response = Invoke-WebRequest -Uri http://localhost:8080/sms -Method POST -ContentType "application/xml" -Body $body
        
        Write-Host "RESPONSE:" -ForegroundColor Green
        Write-Host "Status: $($response.StatusCode) $($response.StatusDescription)"
        Write-Host ""
        Write-Host $response.Content
        Write-Host ""
        
    } catch {
        Write-Host "ERROR: $_" -ForegroundColor Red
    }
    
}

Write-Host "All Tests Complete!" -ForegroundColor Cyan

