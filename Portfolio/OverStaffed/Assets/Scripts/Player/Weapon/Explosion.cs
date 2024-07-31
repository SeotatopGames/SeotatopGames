using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class Explosion : MonoBehaviour
{
    [SerializeField] int damage;
    [SerializeField] int pushAmount;
    [SerializeField] bool doesBurn;
    [SerializeField] float burnDuration;


    // Start is called before the first frame update
    void Start()
    {
        Destroy(gameObject, 1);
    }

    private void OnTriggerEnter(Collider other)
    {
        if (other.GetComponent<IPhysics>() != null)
        {
            IPhysics physicsable = other.GetComponent<IPhysics>();

            Vector3 dir = other.transform.position - transform.position;
            physicsable.Knockback(dir * pushAmount);
        }

        if (other.GetComponent<IDamage>() != null)
        {
            IDamage damageable = other.GetComponent<IDamage>();

            damageable.TakeDamage(damage);
            if(doesBurn)
            {
                damageable.Burn(burnDuration, 1);
            }
        }
    }
}
