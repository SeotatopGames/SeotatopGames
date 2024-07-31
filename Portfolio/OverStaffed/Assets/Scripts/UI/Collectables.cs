using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class Collectables : MonoBehaviour
{
    [SerializeField] float value;
    enum PickupType
    {
        Health,
        UltimateCharge,
        Mimic,
        Exp,
        Poison
    }

    [SerializeField] PickupType Pickup;
    [SerializeField] AudioSource collectSound;

    [SerializeField] GameObject mimic;
    /*enum CollectType 
    { 
      Health
    }

    [SerializeField] CollectType type;
    */

    private void OnTriggerEnter(Collider other)
    {
        if (other.CompareTag("Player"))
        {
            switch (Pickup)
            {
                case PickupType.Health:
                    if (value >= 0)
                        AudioManager.instance.HealthPickupAudio();
                    else
                        AudioManager.instance.HurtPickUpAudio();
                    gameManager.instance.playerController.TakeDamage((int)-value);
                    break;
                case PickupType.UltimateCharge:
                    gameManager.instance.playerController.ChargeUt(value);
                    break;
                case PickupType.Mimic:
                    Instantiate(mimic, transform.position, mimic.transform.rotation);
                    break;
                case PickupType.Exp:
                    gameManager.instance.playerStats.GainExp((int)value);
                    break;
                case PickupType.Poison:
                    gameManager.instance.playerController.Poison(value, 1);
                    break;
            }
            Destroy(gameObject);
        }
    }
}
