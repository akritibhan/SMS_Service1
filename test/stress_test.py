#!/usr/bin/env python3
"""
Comprehensive TPS test - runs both 1000 and 5000 request tests
"""

import asyncio
import aiohttp
import time
import statistics
import sys

SERVER_URL = 'http://localhost:8080/sms'

# Multiple sample requests to test different senders
SAMPLE_REQUESTS = [
    # BANK001 patterns
    """<?xml version="1.0" encoding="UTF-8"?>
<SMSRequest>
    <Sender>BANK001</Sender>
    <Receiver>9876543210</Receiver>
    <SMSContent>Rs. 5000 debited from account ending 1234</SMSContent>
</SMSRequest>""",
    
    # BANK002 patterns
    """<?xml version="1.0" encoding="UTF-8"?>
<SMSRequest>
    <Sender>BANK002</Sender>
    <Receiver>9876543210</Receiver>
    <SMSContent>Account credited Rs. 10000 on 15-Nov</SMSContent>
</SMSRequest>""",
    
    # ECOMM01 patterns
    """<?xml version="1.0" encoding="UTF-8"?>
<SMSRequest>
    <Sender>ECOMM01</Sender>
    <Receiver>9876543210</Receiver>
    <SMSContent>Order #ABC12345 confirmed</SMSContent>
</SMSRequest>""",
    
    # TRAVEL1 patterns
    """<?xml version="1.0" encoding="UTF-8"?>
<SMSRequest>
    <Sender>TRAVEL1</Sender>
    <Receiver>9876543210</Receiver>
    <SMSContent>Flight booking confirmed PNR ABC123</SMSContent>
</SMSRequest>""",
    
    # HEALTH patterns
    """<?xml version="1.0" encoding="UTF-8"?>
<SMSRequest>
    <Sender>HEALTH</Sender>
    <Receiver>9876543210</Receiver>
    <SMSContent>Appointment confirmed with Dr. Smith on 20-Nov</SMSContent>
</SMSRequest>""",
    
    # FOOD patterns
    """<?xml version="1.0" encoding="UTF-8"?>
<SMSRequest>
    <Sender>FOOD</Sender>
    <Receiver>9876543210</Receiver>
    <SMSContent>Order #12345 confirmed Rs. 450</SMSContent>
</SMSRequest>""",
    
    # TELCO patterns
    """<?xml version="1.0" encoding="UTF-8"?>
<SMSRequest>
    <Sender>TELCO</Sender>
    <Receiver>9876543210</Receiver>
    <SMSContent>Your recharge of Rs. 299 is successful</SMSContent>
</SMSRequest>""",
    
    # INSURE patterns
    """<?xml version="1.0" encoding="UTF-8"?>
<SMSRequest>
    <Sender>INSURE</Sender>
    <Receiver>9876543210</Receiver>
    <SMSContent>Policy ABC123 premium Rs. 5000 due</SMSContent>
</SMSRequest>""",
    
    # EDUCAT patterns
    """<?xml version="1.0" encoding="UTF-8"?>
<SMSRequest>
    <Sender>EDUCAT</Sender>
    <Receiver>9876543210</Receiver>
    <SMSContent>Fee payment Rs. 15000 received</SMSContent>
</SMSRequest>""",
    
    # GOVT patterns
    """<?xml version="1.0" encoding="UTF-8"?>
<SMSRequest>
    <Sender>GOVT</Sender>
    <Receiver>9876543210</Receiver>
    <SMSContent>Application ABC123 submitted</SMSContent>
</SMSRequest>"""
]

async def send_request(session, request_id):
    """Send a single HTTP request"""
    try:
        start_time = time.time()
        
        # Rotate through different sample requests for variety
        sample_request = SAMPLE_REQUESTS[request_id % len(SAMPLE_REQUESTS)]
        
        async with session.post(
            SERVER_URL,
            data=sample_request,
            headers={'Content-Type': 'application/xml'},
            timeout=aiohttp.ClientTimeout(total=15)  # Increased to 15 seconds
        ) as response:
            response_text = await response.text()
            
            end_time = time.time()
            response_time = (end_time - start_time) * 1000
            
            return {
                'success': response.status == 200,
                'response_time': response_time,
                'request_id': request_id,
                'status': response.status
            }
            
    except asyncio.TimeoutError as e:
        return {
            'success': False,
            'response_time': 0,
            'request_id': request_id,
            'error': f'Timeout: {str(e)}'
        }
    except aiohttp.ClientError as e:
        return {
            'success': False,
            'response_time': 0,
            'request_id': request_id,
            'error': f'ClientError: {type(e).__name__} - {str(e)}'
        }
    except Exception as e:
        return {
            'success': False,
            'response_time': 0,
            'request_id': request_id,
            'error': f'{type(e).__name__}: {str(e)}'
        }

async def run_test(total_requests, concurrent_limit, test_name):
    """Run a load test with specified parameters"""
    print("\n" + "=" * 70)
    print(f"{test_name}")
    print("=" * 70)
    print(f"Total Requests: {total_requests}")
    print(f"Concurrent Limit: {concurrent_limit}")
    print(f"Server: {SERVER_URL}")
    print("=" * 70)
    
    start_time = time.time()
    
    # Create session with connection pooling
    connector = aiohttp.TCPConnector(limit=concurrent_limit, limit_per_host=concurrent_limit)
    async with aiohttp.ClientSession(connector=connector) as session:
        # Create all tasks
        tasks = [send_request(session, i) for i in range(total_requests)]
        
        print(f"\nSending {total_requests} concurrent requests...")
        
        # Execute all requests concurrently
        results = await asyncio.gather(*tasks)
    
    end_time = time.time()
    total_time = end_time - start_time
    
    # Process results
    successful = [r for r in results if r['success']]
    failed = [r for r in results if not r['success']]
    
    success_times = [r['response_time'] for r in successful]
    
    # Print results
    print("\n" + "=" * 70)
    print("RESULTS")
    print("=" * 70)
    print(f"Total Time: {total_time:.2f} seconds")
    print(f"Total Requests: {total_requests}")
    print(f"Successful: {len(successful)}")
    print(f"Failed: {len(failed)}")
    print(f"Success Rate: {(len(successful)/total_requests*100):.2f}%")
    print()
    
    actual_tps = len(successful) / total_time
    print(f"Achieved TPS: {actual_tps:.2f}")
    print(f"Target TPS: 500")
    print(f"Performance: {(actual_tps/500*100):.1f}% of target")
    
    print("=" * 70)
    
    # Show failed requests details and categorize errors
    if failed:
        print(f"\nFailed Requests: {len(failed)}")
        
        # Categorize errors
        error_types = {}
        for f in failed:
            error = f.get('error', 'Unknown error')
            # Extract error type
            if 'Cannot connect' in error or 'Connection refused' in error:
                error_type = 'Connection Refused'
            elif 'Timeout' in error or 'timeout' in error:
                error_type = 'Timeout'
            elif 'network name' in error or 'WinError 64' in error:
                error_type = 'Network Error (Connection Lost)'
            elif 'WinError 10061' in error:
                error_type = 'Connection Refused (10061)'
            elif 'WinError 10054' in error:
                error_type = 'Connection Reset (10054)'
            else:
                error_type = error[:50] if len(error) < 50 else error[:50] + '...'
            
            error_types[error_type] = error_types.get(error_type, 0) + 1
        
        print("\nError Breakdown:")
        for error_type, count in sorted(error_types.items(), key=lambda x: x[1], reverse=True):
            print(f"  {error_type}: {count} ({count/len(failed)*100:.1f}%)")
        
        if len(failed) <= 10:
            print("\nSample Failed Requests:")
            for f in failed[:10]:
                error = f.get('error', 'Unknown error')
                print(f"  Request {f['request_id']}: {error}")
    
    return {
        'total_requests': total_requests,
        'successful': len(successful),
        'failed': len(failed),
        'tps': actual_tps,
        'success_rate': len(successful)/total_requests*100,
        'total_time': total_time
    }

async def run_all_tests():
    """Run all test scenarios"""
    print("\n" + "=" * 70)
    print("COMPREHENSIVE TPS TESTING SUITE")
    print("=" * 70)
    print("Target: 500 TPS with high success rate")
    print("=" * 70)
    
    results = []
    
    # Test 1: 1000 requests (baseline performance)
    result1 = await run_test(
        total_requests=1000,
        concurrent_limit=100,
        test_name="TEST 1: Baseline Performance (1000 Requests)"
    )
    results.append(result1)
    
    print("\n\n Waiting 3 seconds before next test...\n")
    await asyncio.sleep(3)
    
    # Test 2: 5000 requests (stress test)
    result2 = await run_test(
        total_requests=6000,
        concurrent_limit=500,
        test_name="TEST 2: Stress Test (5000 Requests)"
    )
    results.append(result2)
    
    # Summary
    print("\n\n" + "=" * 70)
    print("SUMMARY - ALL TESTS")
    print("=" * 70)
    print(f"{'Test':<35} {'Requests':<12} {'Success':<10} {'TPS':<10} {'vs Target':<12}")
    print("-" * 70)
    
    for i, result in enumerate(results, 1):
        test_name = f"Test {i} ({result['total_requests']} req)"
        success_rate = f"{result['success_rate']:.1f}%"
        tps = f"{result['tps']:.1f}"
        vs_target = f"{result['tps']/500*100:.1f}%"
        
        print(f"{test_name:<35} {result['total_requests']:<12} {success_rate:<10} {tps:<10} {vs_target:<12}")
    
    print("=" * 70)
    
    # Final verdict
    print("\nFINAL VERDICT:")
    if results[0]['tps'] >= 500 and results[0]['success_rate'] >= 95:
        print("PASSED: System achieves 500+ TPS target with >95% success rate")
        print(f"   Baseline: {results[0]['tps']:.0f} TPS ({results[0]['success_rate']:.1f}% success)")
    else:
        print("  WARNING: System does not consistently meet 500 TPS target")
    
    if results[1]['success_rate'] >= 90:
        print(f" STRESS TEST: Handles high load well ({results[1]['success_rate']:.1f}% success under 5000 req)")
    else:
        print(f"  STRESS TEST: Degraded performance under extreme load ({results[1]['success_rate']:.1f}% success)")
    
    print("=" * 70 + "\n")

if __name__ == "__main__":
    try:
        asyncio.run(run_all_tests())
    except KeyboardInterrupt:
        print("\n\n Test interrupted by user")
        sys.exit(1)
    except Exception as e:
        print(f"\n\n Error: {e}")
        sys.exit(1)
